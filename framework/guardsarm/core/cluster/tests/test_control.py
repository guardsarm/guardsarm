# Copyright (C) 2015, Wazuh Inc.
# Copyright (C) 2026, GuardSarm.
# Created by Wazuh, Inc. <info@wazuh.com>.
# This program is a free software; you can redistribute it and/or modify it under the terms of GPLv2

import json
import sys
from unittest.mock import patch, MagicMock

import pytest

from guardsarm.core.exception import GuardSarmClusterError

with patch('guardsarm.common.getgrnam'):
    with patch('guardsarm.common.getpwnam'):
        with patch('guardsarm.common.guardsarm_uid'):
            with patch('guardsarm.common.guardsarm_gid'):
                sys.modules['guardsarm.rbac.orm'] = MagicMock()

                from guardsarm.core.cluster import control
                from guardsarm.core.cluster.local_client import LocalClient


async def async_local_client(command, data):
    return None


@pytest.mark.asyncio
async def test_get_nodes():
    """Verify that get_nodes function returns the cluster nodes list."""
    local_client = LocalClient()
    with patch('guardsarm.core.cluster.local_client.LocalClient.execute', side_effect=async_local_client):
        expected_result = {'items': [{'name': 'master'}, {'name': 'worker1'}], 'totalItems': 2}
        with patch('json.loads', return_value=expected_result):
            result = await control.get_nodes(lc=local_client)
            assert result == expected_result

            result_q = await control.get_nodes(lc=local_client, q='name=master')
            assert result_q == {'items': [{'name': 'master'}], 'totalItems': 1}

        with patch('json.loads', return_value=KeyError(1)):
            with pytest.raises(KeyError):
                await control.get_nodes(lc=local_client)

    with patch('guardsarm.core.cluster.local_client.LocalClient.execute', side_effect=[GuardSarmClusterError(3020), 'error']):
        with pytest.raises(GuardSarmClusterError):
            await control.get_nodes(lc=local_client)

        with pytest.raises(json.JSONDecodeError):
            await control.get_nodes(lc=local_client)


@pytest.mark.asyncio
async def test_get_node():
    """Verify that get_node function returns the current node name."""
    local_client = LocalClient()
    with patch('guardsarm.core.cluster.local_client.LocalClient.execute', side_effect=async_local_client):
        expected_result = [{'items': [{'name': 'master'}]}, {'items': []}]
        for expected in expected_result:
            with patch('json.loads', return_value=expected):
                result = await control.get_node(lc=local_client)
                if len(expected['items']) > 0:
                    assert result == expected['items'][0]
                else:
                    assert result == {}

        with patch('json.loads', return_value=KeyError(1)):
            with pytest.raises(KeyError):
                await control.get_node(lc=local_client)

    with patch('guardsarm.core.cluster.local_client.LocalClient.execute', side_effect=[GuardSarmClusterError(3020), 'error']):
        with pytest.raises(GuardSarmClusterError):
            await control.get_node(lc=local_client)

        with pytest.raises(json.JSONDecodeError):
            await control.get_node(lc=local_client)


@pytest.mark.asyncio
async def test_get_health():
    """Verify that get_health function returns the current node health."""
    local_client = LocalClient()
    with patch('guardsarm.core.cluster.local_client.LocalClient.execute', side_effect=async_local_client):
        expected_result = [{'items': [{'name': 'master'}]}, {'items': []}]
        for expected in expected_result:
            with patch('json.loads', return_value=expected):
                result = await control.get_health(lc=local_client)
                assert result == expected

        with patch('json.loads', return_value=KeyError(1)):
            with pytest.raises(KeyError):
                await control.get_health(lc=local_client)

    with patch('guardsarm.core.cluster.local_client.LocalClient.execute', side_effect=[GuardSarmClusterError(3020), 'error']):
        with pytest.raises(GuardSarmClusterError):
            await control.get_health(lc=local_client)

        with pytest.raises(json.JSONDecodeError):
            await control.get_health(lc=local_client)


@pytest.mark.asyncio
async def test_get_agents():
    """Verify that get_agents function returns the health of the agents connected through the current node."""
    local_client = LocalClient()
    with patch('guardsarm.core.cluster.local_client.LocalClient.execute', side_effect=async_local_client):
        expected_result = [{'items': [{'name': 'master'}]}, {'items': []}]
        for expected in expected_result:
            with patch('json.loads', return_value=expected):
                result = await control.get_agents(lc=local_client)
                assert result == expected

        with patch('json.loads', return_value=KeyError(1)):
            with pytest.raises(KeyError):
                await control.get_agents(lc=local_client)

    with patch('guardsarm.core.cluster.local_client.LocalClient.execute', side_effect=[GuardSarmClusterError(3020), 'error']):
        with pytest.raises(GuardSarmClusterError):
            await control.get_agents(lc=local_client)

        with pytest.raises(json.JSONDecodeError):
            await control.get_agents(lc=local_client)


@pytest.mark.asyncio
async def test_get_system_nodes():
    """Verify that get_system_nodes function returns the name of all cluster nodes."""
    with patch('guardsarm.core.cluster.local_client.LocalClient.execute', side_effect=async_local_client):
        expected_result = [{'items': [{'name': 'master'}]}]
        for expected in expected_result:
            with patch('guardsarm.core.cluster.control.get_nodes', return_value=expected):
                result = await control.get_system_nodes()
                assert result == [expected['items'][0]['name']]

    with patch('guardsarm.core.cluster.local_client.LocalClient.execute', side_effect=[GuardSarmClusterError(3020), 'error']):
        with pytest.raises(GuardSarmClusterError):
            await control.get_system_nodes()

        with pytest.raises(json.JSONDecodeError):
            await control.get_system_nodes()


@pytest.mark.asyncio
async def test_get_system_nodes_or_none():
    """Verify that get_system_nodes_or_none function returns the name of all cluster nodes."""
    with patch('guardsarm.core.cluster.local_client.LocalClient.execute', side_effect=async_local_client):
        expected_result = [{'items': [{'name': 'master'}]}]
        for expected in expected_result:
            with patch('guardsarm.core.cluster.control.get_nodes', return_value=expected):
                result = await control.get_system_nodes_or_none()
                assert result == [expected['items'][0]['name']]
