# Copyright (C) 2015, Wazuh Inc.
# Copyright (C) 2026, GuardSarm.
# Created by Wazuh, Inc. <info@wazuh.com>.
# This program is free software; you can redistribute it and/or modify it under the terms of GPLv2

import sys
from json import dumps
from unittest.mock import call, MagicMock, patch

import pytest

with patch('guardsarm.core.common.guardsarm_uid'):
    with patch('guardsarm.core.common.guardsarm_gid'):
        sys.modules['guardsarm.rbac.orm'] = MagicMock()
        import guardsarm.rbac.decorators
        from guardsarm.tests.util import RBAC_bypasser

        del sys.modules['guardsarm.rbac.orm']
        guardsarm.rbac.decorators.expose_resources = RBAC_bypasser

        import guardsarm.stats as stats
        from guardsarm.core.results import AffectedItemsGuardSarmResult
        from api.util import remove_nones_to_dict
        from guardsarm.core.tests.test_agent import InitAgent

SOCKET_PATH_DAEMONS_MAPPING = {'/var/guardsarm-manager/queue/sockets/remote': 'guardsarm-manager-remoted',
                               '/var/guardsarm-manager/queue/sockets/analysis': 'guardsarm-manager-analysisd'}
DAEMON_SOCKET_PATHS_MAPPING = {'guardsarm-manager-remoted': '/var/guardsarm-manager/queue/sockets/remote',
                               'guardsarm-manager-analysisd': '/var/guardsarm-manager/queue/sockets/analysis'}

test_data = InitAgent()


def send_msg_to_wdb(msg, raw=False):
    query = ' '.join(msg.split(' ')[2:])
    result = list(map(remove_nones_to_dict, map(dict, test_data.cur.execute(query).fetchall())))
    return ['ok', dumps(result)] if raw else result


@patch('guardsarm.core.common.REMOTED_SOCKET', '/var/guardsarm-manager/queue/sockets/remote')
@patch('guardsarm.core.common.ANALYSISD_SOCKET', '/var/guardsarm-manager/queue/sockets/analysis')
@patch('guardsarm.core.common.WDB_SOCKET', '/var/guardsarm-manager/queue/db/wdb')
@patch('guardsarm.stats.get_daemons_stats_socket')
def test_get_daemons_stats(mock_get_daemons_stats_socket):
    """Makes sure get_daemons_stats() fit with the expected."""
    response = stats.get_daemons_stats(['guardsarm-manager-remoted', 'guardsarm-manager-analysisd', 'guardsarm-manager-db'])

    calls = [call('/var/guardsarm-manager/queue/sockets/remote'), call('/var/guardsarm-manager/queue/sockets/analysis'),
             call('/var/guardsarm-manager/queue/db/wdb')]
    mock_get_daemons_stats_socket.assert_has_calls(calls)
    assert isinstance(response, AffectedItemsGuardSarmResult), 'The result is not AffectedItemsGuardSarmResult type'
    assert response.total_affected_items == len(response.affected_items)


@patch('guardsarm.core.common.REMOTED_SOCKET', '/var/guardsarm-manager/queue/sockets/wrong_socket_name')
def test_get_daemons_stats_ko():
    """Makes sure get_daemons_stats() fit with the expected."""
    response = stats.get_daemons_stats(['guardsarm-manager-remoted'])

    assert isinstance(response, AffectedItemsGuardSarmResult), 'The result is not AffectedItemsGuardSarmResult type'
    assert response.render()['data']['failed_items'][0]['error']['code'] == 1121, 'Expected error code was not returned'


def side_effect_test_get_daemons_stats(daemon_path, agents_list):
    return {'name': SOCKET_PATH_DAEMONS_MAPPING[daemon_path], 'agents': [{'id': a} for a in agents_list]}


@pytest.mark.parametrize('daemons_list, expected_daemons_list', [
    ([], ['guardsarm-manager-remoted', 'guardsarm-manager-analysisd']),
    (['guardsarm-manager-remoted'], ['guardsarm-manager-remoted']),
    (['guardsarm-manager-remoted', 'guardsarm-manager-analysisd'], ['guardsarm-manager-remoted', 'guardsarm-manager-analysisd'])
])
@patch('guardsarm.core.wdb.GuardSarmDBConnection._send', side_effect=send_msg_to_wdb)
@patch('socket.socket.connect')
@patch('guardsarm.stats.get_agents_info', return_value={'001', '002', '003', '004', '005'})
@patch('guardsarm.core.common.REMOTED_SOCKET', '/var/guardsarm-manager/queue/sockets/remote')
@patch('guardsarm.core.common.ANALYSISD_SOCKET', '/var/guardsarm-manager/queue/sockets/analysis')
@patch('guardsarm.stats.get_daemons_stats_socket', side_effect=side_effect_test_get_daemons_stats)
def test_get_daemons_stats_agents(mock_get_daemons_stats_socket, mock_get_agents_info, mock_socket_connect,
                                  mock_send_wdb, daemons_list, expected_daemons_list):
    """Makes sure get_daemons_stats_agents() fit with the expected."""
    agents_list = ['001', '004', '999']  # Only stats from 001 are obtained
    expected_errors_and_items = {'1701': {'999'}, '1707': {'004'}}
    result = stats.get_daemons_stats_agents(daemons_list, agents_list)

    # get_daemons_stats_socket called with the expected parameters
    calls = [call(DAEMON_SOCKET_PATHS_MAPPING[daemon], agents_list=[1]) for daemon in expected_daemons_list]
    mock_get_daemons_stats_socket.assert_has_calls(calls)

    # Check affected_items
    assert result.affected_items == [{'name': daemon, 'agents': [{'id': 1}]} for daemon in expected_daemons_list]
    assert result.total_affected_items == len(expected_daemons_list)

    # Check failed items
    error_codes_in_failed_items = [error.code for error in result.failed_items.keys()]
    failed_items = list(result.failed_items.values())
    errors_and_items = {str(error): failed_items[i] for i, error in enumerate(error_codes_in_failed_items)}
    assert expected_errors_and_items == errors_and_items

    assert isinstance(result, AffectedItemsGuardSarmResult), 'The result is not an AffectedItemsGuardSarmResult object'


def side_effect_test_get_daemons_stats_all(daemon_path, agents_list, last_id):
    # side_effect used to return a response with 10 items and 'due' the first time that get_daemons_stats_socket is
    # called, and a response with 10 items and 'ok' the second time
    if last_id:
        last_id += 1
    return {'data': {'name': SOCKET_PATH_DAEMONS_MAPPING[daemon_path],
                     'agents': [{'id': i} for i in range(last_id, last_id + 10)]},
            'message': 'due' if last_id == 0 else 'ok',
            'error': 1 if last_id == 0 else 0}


@pytest.mark.parametrize('daemons_list, expected_daemons_list', [
    ([], ['guardsarm-manager-remoted', 'guardsarm-manager-analysisd']),
    (['guardsarm-manager-remoted'], ['guardsarm-manager-remoted']),
    (['guardsarm-manager-remoted', 'guardsarm-manager-analysisd'], ['guardsarm-manager-remoted', 'guardsarm-manager-analysisd'])
])
@patch('guardsarm.core.common.REMOTED_SOCKET', '/var/guardsarm-manager/queue/sockets/remote')
@patch('guardsarm.core.common.ANALYSISD_SOCKET', '/var/guardsarm-manager/queue/sockets/analysis')
@patch('guardsarm.stats.get_daemons_stats_socket', side_effect=side_effect_test_get_daemons_stats_all)
def test_get_daemons_stats_all_agents(mock_get_daemons_stats_socket, daemons_list, expected_daemons_list):
    """Makes sure get_daemons_stats_agents() fit with the expected."""
    result = stats.get_daemons_stats_agents(daemons_list, ['all'])

    # get_daemons_stats_socket called with the expected parameters
    calls = []
    for daemon in expected_daemons_list:
        calls.extend((call(DAEMON_SOCKET_PATHS_MAPPING[daemon], agents_list='all', last_id=0),
                      call(DAEMON_SOCKET_PATHS_MAPPING[daemon], agents_list='all', last_id=9)))
    mock_get_daemons_stats_socket.assert_has_calls(calls)

    # Check affected_items
    expected_affected_items = [{'name': daemon, 'agents': [{'id': i} for i in range(0, 20)]} for daemon in
                               expected_daemons_list]
    assert result.affected_items == expected_affected_items
    assert result.total_affected_items == len(expected_daemons_list)

    # Check failed items
    assert not result.failed_items

    assert isinstance(result, AffectedItemsGuardSarmResult), 'The result is not an AffectedItemsGuardSarmResult object'


@pytest.mark.parametrize('component', [
    'logcollector', 'test'
])
@patch('guardsarm.core.agent.Agent.get_stats')
@patch('guardsarm.stats.get_agents_info', return_value=['001', '002'])
def test_get_agents_component_stats_json(mock_agents_info, mock_getstats, component):
    """Test `get_agents_component_stats_json` function from agent module."""
    response = stats.get_agents_component_stats_json(agent_list=['001'], component=component)
    assert isinstance(response, AffectedItemsGuardSarmResult), 'The result is not AffectedItemsGuardSarmResult type'
    mock_getstats.assert_called_once_with(component=component)


@patch('guardsarm.core.agent.Agent.get_stats')
@patch('guardsarm.stats.get_agents_info', return_value=['001', '002'])
def test_get_agents_component_stats_json_ko(mock_agents_info, mock_getstats):
    """Test `get_agents_component_stats_json` function from agent module."""
    response = stats.get_agents_component_stats_json(agent_list=['003'], component='logcollector')
    assert isinstance(response, AffectedItemsGuardSarmResult), 'The result is not AffectedItemsGuardSarmResult type'
    assert response.render()['data']['failed_items'][0]['error']['code'] == 1701, 'Expected error code was not returned'


METRICS_DUMP_DATA = {
    "status": 0,
    "name": "engine",
    "uptime": 99,
    "global": [{"name": "router.queue.size", "type": 0, "enabled": True, "value": 500}],
    "spaces": [],
}


@patch('guardsarm.stats.EngineHTTPClient')
def test_get_engine_metrics(mock_client_cls):
    mock_client = MagicMock()
    mock_client.get_metrics_dump.return_value = METRICS_DUMP_DATA
    mock_client_cls.return_value = mock_client

    response = stats.get_engine_metrics()

    assert isinstance(response, AffectedItemsGuardSarmResult)
    assert response.total_affected_items == 1
    assert response.affected_items[0] == METRICS_DUMP_DATA
    assert not response.failed_items


@patch('guardsarm.stats.EngineHTTPClient')
def test_get_engine_metrics_ko(mock_client_cls):
    from guardsarm.core.exception import GuardSarmInternalError

    mock_client = MagicMock()
    mock_client.get_metrics_dump.side_effect = GuardSarmInternalError(2021)
    mock_client_cls.return_value = mock_client

    response = stats.get_engine_metrics()

    assert isinstance(response, AffectedItemsGuardSarmResult)
    assert response.total_affected_items == 0
    assert response.render()['data']['failed_items'][0]['error']['code'] == 2021
