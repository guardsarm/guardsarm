# Copyright (C) 2015, Wazuh Inc.
# Copyright (C) 2026, GuardSarm.
# Created by Wazuh, Inc. <info@wazuh.com>.
# This program is free software; you can redistribute it and/or modify it under the terms of GPLv2

# run test: python3 -m pytest tests/test_utils.py -v --log-cli-level=DEBUG

import logging
import os
import sys
from unittest.mock import MagicMock, patch

import pytest

logger = logging.getLogger(__name__)

sys.path.append(os.path.join(os.path.dirname(os.path.realpath(__file__)), '..'))
import utils


@pytest.fixture(autouse=True)
def clear_lru_caches():
    """Clear all lru_cache caches before each test to avoid state leakage."""
    utils.find_guardsarm_path.cache_clear()
    utils.get_guardsarm_version.cache_clear()
    utils.get_guardsarm_revision.cache_clear()
    utils.get_guardsarm_type.cache_clear()
    yield
    utils.find_guardsarm_path.cache_clear()
    utils.get_guardsarm_version.cache_clear()
    utils.get_guardsarm_revision.cache_clear()
    utils.get_guardsarm_type.cache_clear()


# ---------------------------------------------------------------------------
# find_guardsarm_path
# ---------------------------------------------------------------------------

def test_find_guardsarm_path_returns_string():
    """find_guardsarm_path always returns a string."""
    result = utils.find_guardsarm_path()
    logger.info("find_guardsarm_path() type=%s value=%r", type(result).__name__, result)
    assert isinstance(result, str)


def test_find_guardsarm_path_returns_parent_of_wodles():
    """find_guardsarm_path returns the parent directory of the 'wodles' segment, not wodles itself."""
    path = utils.find_guardsarm_path()
    logger.info("find_guardsarm_path() => %r", path)
    assert not path.endswith('wodles')


def test_find_guardsarm_path_no_wodles_in_path():
    """find_guardsarm_path returns '' when __file__ is not under a 'wodles' directory."""
    with patch('utils.os.path.abspath', return_value='/tmp/some/other/dir'):
        with patch('utils.os.path.dirname', return_value='/tmp/some/other/dir'):
            utils.find_guardsarm_path.cache_clear()
            result = utils.find_guardsarm_path()
    logger.info("find_guardsarm_path() with no 'wodles' segment => %r", result)
    assert result == ''


def test_find_guardsarm_path_cache():
    """find_guardsarm_path is only computed once due to lru_cache."""
    with patch('utils.os.path.abspath', return_value='/opt/guardsarm/wodles') as mock_abspath:
        utils.find_guardsarm_path.cache_clear()
        utils.find_guardsarm_path()
        utils.find_guardsarm_path()
        logger.info("os.path.abspath call_count after 2 find_guardsarm_path() calls => %d", mock_abspath.call_count)
        assert mock_abspath.call_count == 1


def test_find_guardsarm_path_relative_path_sentinel():
    """find_guardsarm_path handles a single-component relative path (sentinel for relative paths branch)."""
    # os.path.split('wodles') => ('', 'wodles'), so parts[1] == abs_path triggers the elif branch.
    with patch('utils.os.path.abspath', return_value='wodles'):
        utils.find_guardsarm_path.cache_clear()
        result = utils.find_guardsarm_path()
    # 'wodles' is found at index 0, so range(0, 0) produces no iterations → guardsarm_path == ''
    assert result == ''


# ---------------------------------------------------------------------------
# call_guardsarm_control
# ---------------------------------------------------------------------------

@patch('utils.find_guardsarm_path', return_value='/var/ossec')
@patch('utils.subprocess.Popen')
def test_call_guardsarm_control_returns_stdout(mock_popen, mock_path):
    """call_guardsarm_control returns the decoded stdout of the subprocess."""
    mock_proc = MagicMock()
    mock_proc.communicate.return_value = (b'GUARDSARM_VERSION="5.0.0"\n', None)
    mock_popen.return_value = mock_proc

    result = utils.call_guardsarm_control('info')
    logger.info("call_guardsarm_control('info') => %r", result)
    logger.info("Popen called with => %s", mock_popen.call_args)

    assert result == 'GUARDSARM_VERSION="5.0.0"\n'
    mock_popen.assert_called_once_with(
        ['/var/ossec/bin/guardsarm-control', 'info'],
        stdout=utils.subprocess.PIPE,
    )


@patch('utils.find_guardsarm_path', return_value='/var/ossec')
@patch('utils.subprocess.Popen', side_effect=OSError)
@patch('builtins.print')
def test_call_guardsarm_control_oserror_exits(mock_print, mock_popen, mock_path):
    """call_guardsarm_control prints an error and exits when OSError is raised."""
    with pytest.raises(SystemExit):
        utils.call_guardsarm_control('info')
    logger.info("print called with => %s", mock_print.call_args)
    mock_print.assert_called_once_with(
        'ERROR: a problem occurred while executing /var/ossec/bin/guardsarm-control'
    )


@patch('utils.find_guardsarm_path', return_value='/var/ossec')
@patch('utils.subprocess.Popen', side_effect=ChildProcessError)
@patch('builtins.print')
def test_call_guardsarm_control_childprocesserror_exits(mock_print, mock_popen, mock_path):
    """call_guardsarm_control prints an error and exits when ChildProcessError is raised."""
    with pytest.raises(SystemExit):
        utils.call_guardsarm_control('info')
    logger.info("print called with => %s", mock_print.call_args)
    mock_print.assert_called_once()


# ---------------------------------------------------------------------------
# get_guardsarm_info
# ---------------------------------------------------------------------------

@patch('utils.call_guardsarm_control', return_value='')
def test_get_guardsarm_info_empty_output_returns_error(mock_ctrl):
    """get_guardsarm_info returns 'ERROR' when guardsarm-control produces no output."""
    result = utils.get_guardsarm_info('GUARDSARM_VERSION')
    logger.info("get_guardsarm_info('GUARDSARM_VERSION') with empty output => %r", result)
    assert result == 'ERROR'


@patch('utils.call_guardsarm_control', return_value='GUARDSARM_VERSION="5.0.0"\nGUARDSARM_REVISION="1"\n')
def test_get_guardsarm_info_no_field_returns_full_output(mock_ctrl):
    """get_guardsarm_info returns the full output when field is empty."""
    result = utils.get_guardsarm_info('')
    logger.info("get_guardsarm_info('') => %r", result)
    assert 'GUARDSARM_VERSION' in result
    assert 'GUARDSARM_REVISION' in result


@patch('utils.call_guardsarm_control', return_value='GUARDSARM_VERSION="5.0.0"\nGUARDSARM_REVISION="1"\nGUARDSARM_TYPE="server"\n')
def test_get_guardsarm_info_version_field(mock_ctrl):
    """get_guardsarm_info correctly parses and returns GUARDSARM_VERSION."""
    result = utils.get_guardsarm_info('GUARDSARM_VERSION')
    logger.info("get_guardsarm_info('GUARDSARM_VERSION') => %r", result)
    assert result == '5.0.0'


@patch('utils.call_guardsarm_control', return_value='GUARDSARM_VERSION="5.0.0"\nGUARDSARM_REVISION="1"\nGUARDSARM_TYPE="server"\n')
def test_get_guardsarm_info_revision_field(mock_ctrl):
    """get_guardsarm_info correctly parses and returns GUARDSARM_REVISION."""
    result = utils.get_guardsarm_info('GUARDSARM_REVISION')
    logger.info("get_guardsarm_info('GUARDSARM_REVISION') => %r", result)
    assert result == '1'


@patch('utils.call_guardsarm_control', return_value='GUARDSARM_VERSION="5.0.0"\nGUARDSARM_REVISION="1"\nGUARDSARM_TYPE="server"\n')
def test_get_guardsarm_info_type_field(mock_ctrl):
    """get_guardsarm_info correctly parses and returns GUARDSARM_TYPE."""
    result = utils.get_guardsarm_info('GUARDSARM_TYPE')
    logger.info("get_guardsarm_info('GUARDSARM_TYPE') => %r", result)
    assert result == 'server'


@patch('utils.call_guardsarm_control', return_value='GUARDSARM_VERSION="5.0.0"\nGUARDSARM_REVISION="1"\nGUARDSARM_TYPE="server"\n')
def test_get_guardsarm_info_unknown_field_raises_keyerror(mock_ctrl):
    """get_guardsarm_info raises KeyError for an unknown field."""
    with pytest.raises(KeyError) as exc_info:
        utils.get_guardsarm_info('UNKNOWN_FIELD')
    logger.info("get_guardsarm_info('UNKNOWN_FIELD') raised KeyError => %s", exc_info.value)


# ---------------------------------------------------------------------------
# get_guardsarm_version / get_guardsarm_revision / get_guardsarm_type
# ---------------------------------------------------------------------------

@patch('utils.get_guardsarm_info', return_value='5.0.0')
def test_get_guardsarm_version_calls_correct_field(mock_info):
    """get_guardsarm_version delegates to get_guardsarm_info with 'GUARDSARM_VERSION'."""
    result = utils.get_guardsarm_version()
    logger.info("get_guardsarm_version() => %r, get_guardsarm_info called with => %s", result, mock_info.call_args)
    assert result == '5.0.0'
    mock_info.assert_called_once_with('GUARDSARM_VERSION')


@patch('utils.get_guardsarm_info', return_value='1')
def test_get_guardsarm_revision_calls_correct_field(mock_info):
    """get_guardsarm_revision delegates to get_guardsarm_info with 'GUARDSARM_REVISION'."""
    result = utils.get_guardsarm_revision()
    logger.info("get_guardsarm_revision() => %r, get_guardsarm_info called with => %s", result, mock_info.call_args)
    assert result == '1'
    mock_info.assert_called_once_with('GUARDSARM_REVISION')


@patch('utils.get_guardsarm_info', return_value='server')
def test_get_guardsarm_type_calls_correct_field(mock_info):
    """get_guardsarm_type delegates to get_guardsarm_info with 'GUARDSARM_TYPE'."""
    result = utils.get_guardsarm_type()
    logger.info("get_guardsarm_type() => %r, get_guardsarm_info called with => %s", result, mock_info.call_args)
    assert result == 'server'
    mock_info.assert_called_once_with('GUARDSARM_TYPE')


@patch('utils.get_guardsarm_info', return_value='5.0.0')
def test_get_guardsarm_version_cache(mock_info):
    """get_guardsarm_version only calls get_guardsarm_info once due to lru_cache."""
    utils.get_guardsarm_version.cache_clear()
    utils.get_guardsarm_version()
    utils.get_guardsarm_version()
    logger.info("get_guardsarm_info call_count after 2 get_guardsarm_version() calls => %d", mock_info.call_count)
    assert mock_info.call_count == 1


# ---------------------------------------------------------------------------
# Module-level constants
# ---------------------------------------------------------------------------

def test_max_event_size_value():
    """MAX_EVENT_SIZE must equal 65535."""
    logger.info("MAX_EVENT_SIZE => %d", utils.MAX_EVENT_SIZE)
    assert utils.MAX_EVENT_SIZE == 65535


def test_analysisd_path_suffix():
    """ANALYSISD must end with 'queue/sockets/queue'."""
    logger.info("ANALYSISD => %r", utils.ANALYSISD)
    assert utils.ANALYSISD.endswith(os.path.join('queue', 'sockets', 'queue'))
