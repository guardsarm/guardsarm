# Copyright (C) 2015, Wazuh Inc.
# Copyright (C) 2026, GuardSarm.
# Created by Wazuh, Inc. <info@wazuh.com>.
# This program is a free software; you can redistribute it and/or modify it under the terms of GPLv2

from unittest.mock import patch, MagicMock
import pytest

from guardsarm.core.exception import GuardSarmException
from guardsarm.core.guardsarm_socket import GuardSarmSocket, GuardSarmSocketJSON, \
    SOCKET_COMMUNICATION_PROTOCOL_VERSION, create_guardsarm_socket_message


@patch('guardsarm.core.guardsarm_socket.GuardSarmSocket._connect')
def test_GuardSarmSocket__init__(mock_conn):
    """Tests GuardSarmSocket.__init__ function works"""

    GuardSarmSocket('test_path')

    mock_conn.assert_called_once_with()


@patch('guardsarm.core.guardsarm_socket.socket.socket.connect')
def test_GuardSarmSocket_protected_connect(mock_conn):
    """Tests GuardSarmSocket._connect function works"""

    GuardSarmSocket('test_path')

    mock_conn.assert_called_with('test_path')


@patch('guardsarm.core.guardsarm_socket.socket.socket.connect', side_effect=Exception)
def test_GuardSarmSocket_protected_connect_ko(mock_conn):
    """Tests GuardSarmSocket._connect function exceptions works"""

    with pytest.raises(GuardSarmException, match=".* 1013 .*"):
        GuardSarmSocket('test_path')


@patch('guardsarm.core.guardsarm_socket.socket.socket.connect')
@patch('guardsarm.core.guardsarm_socket.socket.socket.close')
def test_GuardSarmSocket_close(mock_close, mock_conn):
    """Tests GuardSarmSocket.close function works"""

    queue = GuardSarmSocket('test_path')

    queue.close()

    mock_conn.assert_called_once_with('test_path')
    mock_close.assert_called_once_with()


@patch('guardsarm.core.guardsarm_socket.socket.socket.connect')
@patch('guardsarm.core.guardsarm_socket.socket.socket.send')
def test_GuardSarmSocket_send(mock_send, mock_conn):
    """Tests GuardSarmSocket.send function works"""

    queue = GuardSarmSocket('test_path')

    response = queue.send(b"\x00\x01")

    assert isinstance(response, MagicMock)
    mock_conn.assert_called_once_with('test_path')


@pytest.mark.parametrize('msg, effect, send_effect, expected_exception', [
    ('text_msg', 'side_effect', None, 1105),
    (b"\x00\x01", 'return_value', 0, 1014),
    (b"\x00\x01", 'side_effect', Exception, 1014)
])
@patch('guardsarm.core.guardsarm_socket.socket.socket.connect')
def test_GuardSarmSocket_send_ko(mock_conn, msg, effect, send_effect, expected_exception):
    """Tests GuardSarmSocket.send function exceptions works"""

    queue = GuardSarmSocket('test_path')

    if effect == 'return_value':
        with patch('guardsarm.core.guardsarm_socket.socket.socket.send', return_value=send_effect):
            with pytest.raises(GuardSarmException, match=f'.* {expected_exception} .*'):
                queue.send(msg)
    else:
        with patch('guardsarm.core.guardsarm_socket.socket.socket.send', side_effect=send_effect):
            with pytest.raises(GuardSarmException, match=f'.* {expected_exception} .*'):
                queue.send(msg)

    mock_conn.assert_called_once_with('test_path')


@patch('guardsarm.core.guardsarm_socket.socket.socket.connect')
@patch('guardsarm.core.guardsarm_socket.unpack', return_value='1024')
@patch('guardsarm.core.guardsarm_socket.socket.socket.recv')
def test_GuardSarmSocket_receive(mock_recv, mock_unpack, mock_conn):
    """Tests GuardSarmSocket.receive function works"""

    queue = GuardSarmSocket('test_path')

    response = queue.receive()

    assert isinstance(response, MagicMock)
    mock_conn.assert_called_once_with('test_path')


@patch('guardsarm.core.guardsarm_socket.socket.socket.connect')
@patch('guardsarm.core.guardsarm_socket.socket.socket.recv', side_effect=Exception)
def test_GuardSarmSocket_receive_ko(mock_recv, mock_conn):
    """Tests GuardSarmSocket.receive function exception works"""

    queue = GuardSarmSocket('test_path')

    with pytest.raises(GuardSarmException, match=".* 1014 .*"):
        queue.receive()

    mock_conn.assert_called_once_with('test_path')


@patch('guardsarm.core.guardsarm_socket.GuardSarmSocket._connect')
def test_GuardSarmSocketJSON__init__(mock_conn):
    """Tests GuardSarmSocketJSON.__init__ function works"""

    GuardSarmSocketJSON('test_path')

    mock_conn.assert_called_once_with()


@patch('guardsarm.core.guardsarm_socket.socket.socket.connect')
@patch('guardsarm.core.guardsarm_socket.GuardSarmSocket.send')
def test_GuardSarmSocketJSON_send(mock_send, mock_conn):
    """Tests GuardSarmSocketJSON.send function works"""

    queue = GuardSarmSocketJSON('test_path')

    response = queue.send('test_msg')

    assert isinstance(response, MagicMock)
    mock_conn.assert_called_once_with('test_path')


@pytest.mark.parametrize('raw', [
    True, False
])
@patch('guardsarm.core.guardsarm_socket.socket.socket.connect')
@patch('guardsarm.core.guardsarm_socket.GuardSarmSocket.receive')
@patch('guardsarm.core.guardsarm_socket.loads', return_value={'error':0, 'message':None, 'data':'Ok'})
def test_GuardSarmSocketJSON_receive(mock_loads, mock_receive, mock_conn, raw):
    """Tests GuardSarmSocketJSON.receive function works"""
    queue = GuardSarmSocketJSON('test_path')
    response = queue.receive(raw=raw)
    if raw:
        assert isinstance(response, dict)
    else:
        assert isinstance(response, str)
    mock_conn.assert_called_once_with('test_path')


@patch('guardsarm.core.guardsarm_socket.socket.socket.connect')
@patch('guardsarm.core.guardsarm_socket.GuardSarmSocket.receive')
@patch('guardsarm.core.guardsarm_socket.loads', return_value={'error':10000, 'message':'Error', 'data':'KO'})
def test_GuardSarmSocketJSON_receive_ko(mock_loads, mock_receive, mock_conn):
    """Tests GuardSarmSocketJSON.receive function works"""

    queue = GuardSarmSocketJSON('test_path')

    with pytest.raises(GuardSarmException, match=".* 10000 .*"):
        queue.receive()

    mock_conn.assert_called_once_with('test_path')


@pytest.mark.parametrize('origin, command, parameters', [
    ('origin_sample', 'command_sample', {'sample': 'sample'}),
    (None, 'command_sample', {'sample': 'sample'}),
    ('origin_sample', None, {'sample': 'sample'}),
    ('origin_sample', 'command_sample', None),
    (None, None, None)
])
def test_create_guardsarm_socket_message(origin, command, parameters):
    """Test create_guardsarm_socket_message function."""
    response_message = create_guardsarm_socket_message(origin, command, parameters)
    assert response_message['version'] == SOCKET_COMMUNICATION_PROTOCOL_VERSION
    assert response_message.get('origin') == origin
    assert response_message.get('command') == command
    assert response_message.get('parameters') == parameters
