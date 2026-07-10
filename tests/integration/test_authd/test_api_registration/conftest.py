"""
Copyright (C) 2026 GuardSarm, Inc.
Created by Wazuh, Inc. <info@wazuh.com>.
Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
"""
import pytest

from guardsarm_testing.constants.paths.logs import GUARDSARM_API_LOG_FILE_PATH, GUARDSARM_API_JSON_LOG_FILE_PATH
from guardsarm_testing.utils.callbacks import generate_callback
from guardsarm_testing.tools.monitors import file_monitor
from guardsarm_testing.constants.api import GUARDSARM_API_PORT
from guardsarm_testing.modules.api.patterns import API_STARTED_MSG
from guardsarm_testing.utils import configuration as guardsarm_config


@pytest.fixture(scope='module')
def configure_for_api_test():
    """Write an auth-enabled configuration so authd is running when daemons restart for API tests."""
    backup = guardsarm_config.get_guardsarm_conf()
    config_with_auth = guardsarm_config.set_section_guardsarm_conf([{
        'section': 'auth',
        'elements': [
            {'disabled': {'value': 'no'}},
            {'remote_enrollment': {'value': 'yes'}},
        ]
    }])
    guardsarm_config.write_guardsarm_conf(config_with_auth)
    yield
    guardsarm_config.write_guardsarm_conf(backup)


@pytest.fixture(scope='module')
def wait_for_api_startup_module():
    """Monitor the API log file to detect whether it has been started or not.

    Raises:
        RuntimeError: When the log was not found.
    """
    # Set the default values
    logs_format = 'plain'
    host = ['0.0.0.0', '::']
    port = GUARDSARM_API_PORT

    # Check if specific values were set or set the defaults
    file_to_monitor = GUARDSARM_API_JSON_LOG_FILE_PATH if logs_format == 'json' else GUARDSARM_API_LOG_FILE_PATH
    monitor_start_message = file_monitor.FileMonitor(file_to_monitor)
    monitor_start_message.start(
        callback=generate_callback(API_STARTED_MSG, {
            'host': str(host),
            'port': str(port)
        })
    )

    if monitor_start_message.callback_result is None:
        raise RuntimeError('The API was not started as expected.')
