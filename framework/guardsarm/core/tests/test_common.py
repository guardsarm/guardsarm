# Copyright (C) 2015, Wazuh Inc.
# Created by Wazuh, Inc. <info@wazuh.com>.
# This program is a free software; you can redistribute it and/or modify it under the terms of GPLv2

from grp import getgrnam
from pwd import getpwnam
from unittest.mock import patch

import pytest

from guardsarm.core.common import find_guardsarm_path, guardsarm_uid, guardsarm_gid, context_cached, reset_context_cache


@pytest.mark.parametrize('fake_path, expected', [
    ('/var/guardsarm-manager/framework/python/lib/python3.7/site-packages/guardsarm-3.10.0-py3.7.egg/guardsarm', '/var/guardsarm-manager'),
    ('/my/custom/path/framework/python/lib/python3.7/site-packages/guardsarm-3.10.0-py3.7.egg/guardsarm', '/my/custom/path'),
    ('/my/fake/path', '')
])
def test_find_guardsarm_path(fake_path, expected):
    with patch('guardsarm.core.common.__file__', new=fake_path):
        assert (find_guardsarm_path.__wrapped__() == expected)


def test_find_guardsarm_path_relative_path():
    with patch('os.path.abspath', return_value='~/framework'):
        assert (find_guardsarm_path.__wrapped__() == '~')


def test_guardsarm_uid():
    with patch('guardsarm.core.common.getpwnam', return_value=getpwnam("root")):
        guardsarm_uid()


def test_guardsarm_gid():
    with patch('guardsarm.core.common.getgrnam', return_value=getgrnam("root")):
        guardsarm_gid()


def test_context_cached():
    """Verify that context_cached decorator correctly saves and returns saved value when called again."""

    test_context_cached.calls_to_foo = 0

    @context_cached('foobar')
    def foo(arg='bar', **data):
        test_context_cached.calls_to_foo += 1
        return arg

    # The result of function 'foo' is being cached and it has been called once
    assert foo() == 'bar' and test_context_cached.calls_to_foo == 1, '"bar" should be returned with 1 call to foo.'
    assert foo() == 'bar' and test_context_cached.calls_to_foo == 1, '"bar" should be returned with 1 call to foo.'

    # foo called with an argument
    assert foo('other_arg') == 'other_arg' and test_context_cached.calls_to_foo == 2, '"other_arg" should be ' \
                                                                                      'returned with 2 calls to foo. '

    # foo called with the same argument as default, a new context var is created in the cache
    assert foo('bar') == 'bar' and test_context_cached.calls_to_foo == 3, '"bar" should be returned with 3 calls to ' \
                                                                          'foo. '

    # Reset cache and calls to foo
    reset_context_cache()
    test_context_cached.calls_to_foo = 0

    # foo called with kwargs, a new context var is created with kwargs not empty
    assert foo(data='bar') == 'bar' and test_context_cached.calls_to_foo == 1, '"bar" should be returned with 1 ' \
                                                                               'calls to foo. '


@patch('guardsarm.core.stats.guardsarm_socket.create_guardsarm_socket_message', side_effect=SystemExit)
def test_origin_module_context_var_framework(mock_create_socket_msg):
    """Test that the origin_module context variable is being set to framework."""
    from guardsarm.core import stats

    # side_effect used to avoid mocking the rest of functions
    with pytest.raises(SystemExit):
        stats.get_daemons_stats_socket(None)

    assert mock_create_socket_msg.call_args[1]['origin']['module'] == 'framework'


@pytest.mark.asyncio
@patch('guardsarm.core.stats.guardsarm_socket.create_guardsarm_socket_message', side_effect=SystemExit)
@patch('guardsarm.core.cluster.dapi.dapi.DistributedAPI.check_guardsarm_status', side_effect=None)
async def test_origin_module_context_var_api(mock_check_guardsarm_status, mock_create_socket_msg):
    """Test that the origin_module context variable is being set to API."""
    import logging
    from guardsarm.core.cluster.dapi import dapi
    from guardsarm import stats

    # side_effect used to avoid mocking the rest of functions
    with pytest.raises(SystemExit):
        d = dapi.DistributedAPI(f=stats.get_daemons_stats, logger=logging.getLogger('guardsarm'), is_async=True)
        await d.distribute_function()

    assert mock_create_socket_msg.call_args[1]['origin']['module'] == 'API'
