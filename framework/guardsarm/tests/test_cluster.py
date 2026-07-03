# Copyright (C) 2015, Wazuh Inc.
# Created by Wazuh, Inc. <info@wazuh.com>.
# This program is a free software; you can redistribute it and/or modify it under the terms of GPLv2

import sys
from unittest.mock import MagicMock, patch

import pytest

with patch('guardsarm.core.common.guardsarm_uid'):
    with patch('guardsarm.core.common.guardsarm_gid'):
        sys.modules['guardsarm.rbac.orm'] = MagicMock()
        import guardsarm.rbac.decorators

        del sys.modules['guardsarm.rbac.orm']

        from guardsarm.tests.util import RBAC_bypasser

        guardsarm.rbac.decorators.expose_resources = RBAC_bypasser
        from guardsarm import cluster
        from guardsarm.core import common
        from guardsarm.core.exception import GuardSarmError, GuardSarmResourceNotFound
        from guardsarm.core.cluster.local_client import LocalClient
        from guardsarm.core.results import GuardSarmResult

default_config = {'node_type': 'master', 'name': 'guardsarm', 'node_name': 'node01',
                  'key': '', 'port': 1516, 'bind_addr': '127.0.0.1', 'nodes': ['127.0.0.1'], 'hidden': 'no'}


@patch('guardsarm.cluster.read_config', return_value=default_config)
def test_read_config_wrapper(mock_read_config):
    """Verify that the read_config_wrapper returns the default configuration."""
    result = cluster.read_config_wrapper()
    assert result.affected_items == [default_config]


@patch('guardsarm.cluster.read_config', side_effect=GuardSarmError(1001))
def test_read_config_wrapper_exception(mock_read_config):
    """Verify the exceptions raised in read_config_wrapper."""
    result = cluster.read_config_wrapper()
    assert list(result.failed_items.keys())[0] == GuardSarmError(1001)


@patch('guardsarm.cluster.get_node', return_value={'cluster': default_config["name"],
                                               'node': default_config["node_name"],
                                               'type': default_config["node_type"]})
def test_node_wrapper(mock_get_node):
    """Verify that the node_wrapper returns the default node information."""
    result = cluster.get_node_wrapper()
    assert result.affected_items == [{'cluster': default_config["name"],
                                      'node': default_config["node_name"],
                                      'type': default_config["node_type"]}]


@patch('guardsarm.cluster.get_node', side_effect=GuardSarmError(1001))
def test_node_wrapper_exception(mock_get_node):
    """Verify the exceptions raised in get_node_wrapper."""
    result = cluster.get_node_wrapper()
    assert list(result.failed_items.keys())[0] == GuardSarmError(1001)


def test_get_status_json():
    """Verify that get_status_json returns the default status information."""
    result = cluster.get_status_json()
    expected = GuardSarmResult({'data': {"running": "no"}})
    assert result == expected


@pytest.mark.asyncio
@patch('guardsarm.core.cluster.local_client.LocalClient.start', side_effect=None)
async def test_get_health_nodes(mock_unix_connection):
    """Verify that get_health_nodes returns the health of all nodes."""

    async def async_mock(lc=None, filter_node=None):
        return {'nodes': {'manager': {'info': {'name': 'master'}}}}

    local_client = LocalClient()
    with patch('guardsarm.cluster.get_health', side_effect=async_mock):
        result = await cluster.get_health_nodes(lc=local_client)
    expected = await async_mock()

    assert result.affected_items == [expected['nodes']['manager']]


@pytest.mark.asyncio
async def test_get_nodes_info():
    """Verify that get_nodes_info returns the information of all nodes."""

    async def valid_node(lc=None, filter_node=None):
        return {'items': ['master', 'worker1'], 'totalItems': 2}

    local_client = LocalClient()
    common.cluster_nodes.set(['master', 'worker1', 'worker2'])
    with patch('guardsarm.cluster.get_nodes', side_effect=valid_node):
        result = await cluster.get_nodes_info(lc=local_client, filter_node=['master', 'worker1', 'noexists'])
    expected = await valid_node()

    assert result.affected_items == expected['items']
    assert result.total_affected_items == expected['totalItems']
    assert result.failed_items[GuardSarmResourceNotFound(1730)] == {'noexists'}
    assert result.total_failed_items == 1
