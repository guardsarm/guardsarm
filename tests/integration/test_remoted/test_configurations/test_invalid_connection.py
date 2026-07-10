"""
 Copyright (C) 2026 GuardSarm, Inc.
 Created by Wazuh, Inc. <info@wazuh.com>.
 Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
"""

import os
import pytest

from pathlib import Path
from guardsarm_testing.constants.paths.configurations import GUARDSARM_CONF_PATH
from guardsarm_testing.tools.monitors.file_monitor import FileMonitor
from guardsarm_testing.utils.callbacks import generate_callback
from guardsarm_testing.utils.configuration import get_test_cases_data, load_configuration_template
from guardsarm_testing.constants.paths.logs import GUARDSARM_LOG_PATH

from . import CONFIGS_PATH, TEST_CASES_PATH

from guardsarm_testing.modules.remoted.configuration import REMOTED_DEBUG
from guardsarm_testing.modules.remoted.patterns import CONFIGURATION_ERROR, INVALID_ELEMENT_IN_CONFIGURATION


# Set pytest marks.
pytestmark = [pytest.mark.server, pytest.mark.tier(level=1)]

# Cases metadata and its ids.
cases_path = Path(TEST_CASES_PATH, 'cases_invalid_connection.yaml')
config_path = Path(CONFIGS_PATH, 'config_invalid_element.yaml')
test_configuration, test_metadata, cases_ids = get_test_cases_data(cases_path)
test_configuration = load_configuration_template(config_path, test_configuration, test_metadata)

daemons_handler_configuration = {'all_daemons': True}

local_internal_options = {REMOTED_DEBUG: '2'}


# Test function.
@pytest.mark.parametrize('test_configuration, test_metadata',  zip(test_configuration, test_metadata), ids=cases_ids)
def test_invalid_connection(test_configuration, test_metadata, configure_local_internal_options, truncate_monitored_files,
                            set_guardsarm_configuration, restart_guardsarm_expect_error):

    '''
    description: Check if `guardsarm-manager-remoted` fails using invalid 'connection' values and shows the expected error message
                 to inform about it. For this purpose, the test will set a configuration from the module test cases and
                 check if is correct using a FileMonitor catching the errors.

    parameters:
        - test_configuration
            type: dict
            brief: Configuration applied to ossec.conf.
        - test_metadata:
            type: dict
            brief: Test case metadata.
        - truncate_monitored_files:
            type: fixture
            brief: Truncate all the log files and json alerts files before and after the test execution.
        - configure_local_internal_options:
            type: fixture
            brief: Configure the GuardSarm local internal options using the values from `local_internal_options`.
        - daemons_handler:
            type: fixture
            brief: Starts/Restarts the daemons indicated in `daemons_handler_configuration` before each test,
                   once the test finishes, stops the daemons.
        - restart_guardsarm_expect_error
            type: fixture
            brief: Restart service when expected error is None, once the test finishes stops the daemons.
    '''

    log_monitor = FileMonitor(GUARDSARM_LOG_PATH)
    conf_path = os.path.join('etc', os.path.basename(GUARDSARM_CONF_PATH))

    log_monitor.start(callback=generate_callback(INVALID_ELEMENT_IN_CONFIGURATION))
    assert test_metadata['element_type'] in log_monitor.callback_result

    log_monitor.start(callback=generate_callback(regex=CONFIGURATION_ERROR, replacement={"severity": 'ERROR', "path": conf_path}))
    assert log_monitor.callback_result

    log_monitor.start(callback=generate_callback(CONFIGURATION_ERROR.replace('{severity}', 'CRITICAL').replace('{path}', conf_path)))
    assert log_monitor.callback_result
