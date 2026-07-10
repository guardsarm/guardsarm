'''
copyright: Copyright (C) 2026 GuardSarm, Inc.

           Created by Wazuh, Inc. <info@wazuh.com>.

           Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.

type: integration

brief: The 'guardsarm-agentd' program is the client-side daemon that communicates with the server.
       The objective is to check that, with different states in the 'clients.keys' file,
       the agent successfully enrolls after losing connection with the 'guardsarm-manager-remoted' daemon.
       The guardsarm-manager-remoted program is the server side daemon that communicates with the agents.

components:
    - agentd

targets:
    - agent

daemons:
    - guardsarm-agentd
    - guardsarm-manager-authd
    - guardsarm-manager-remoted

os_platform:
    - linux
    - windows

os_version:
    - Arch Linux
    - Amazon Linux 2
    - Amazon Linux 1
    - CentOS 8
    - CentOS 7
    - Debian Buster
    - Red Hat 8
    - Ubuntu Focal
    - Ubuntu Bionic
    - Windows 10
    - Windows Server 2019
    - Windows Server 2016

references:
    - https://documentation.guardsarmsiem.com/current/user-manual/registering/index.html

tags:
    - enrollment
'''
import pytest
from pathlib import Path
import sys

from guardsarm_testing.constants.platforms import WINDOWS
from guardsarm_testing.modules.agentd.configuration import AGENTD_DEBUG, AGENTD_WINDOWS_DEBUG, AGENTD_TIMEOUT
from guardsarm_testing.tools.simulators.authd_simulator import AuthdSimulator
from guardsarm_testing.tools.simulators.remoted_simulator import RemotedSimulator
from guardsarm_testing.utils.configuration import get_test_cases_data, load_configuration_template

from . import CONFIGS_PATH, TEST_CASES_PATH
from utils import wait_connect, wait_enrollment_try

# Marks
pytestmark = [pytest.mark.agent, pytest.mark.linux, pytest.mark.win32, pytest.mark.tier(level=0)]

# Configuration and cases data.
configs_path = Path(CONFIGS_PATH, 'guardsarm_conf.yaml')
cases_path = Path(TEST_CASES_PATH, 'cases_reconnection_protocol.yaml')

# Test configurations.
config_parameters, test_metadata, test_cases_ids = get_test_cases_data(cases_path)
test_configuration = load_configuration_template(configs_path, config_parameters, test_metadata)

if sys.platform == WINDOWS:
    local_internal_options = {AGENTD_WINDOWS_DEBUG: '0'}
else:
    local_internal_options = {AGENTD_DEBUG: '2'}
local_internal_options.update({AGENTD_TIMEOUT: '5'})

daemons_handler_configuration = {'all_daemons': True}

# Tests
@pytest.mark.parametrize('test_configuration, test_metadata', zip(test_configuration, test_metadata), ids=test_cases_ids)
def test_agentd_reconection_enrollment_with_keys(test_metadata, set_guardsarm_configuration, configure_local_internal_options,
                                                 truncate_monitored_files, clean_keys, add_keys, daemons_handler):
    '''
       description: Check how the agent behaves when losing communication with
                 the 'guardsarm-manager-remoted' daemon and a new enrollment is sent to
                 the 'guardsarm-manager-authd' daemon.
                 In this case, the agent starts with keys.

    guardsarm_min_version: 4.2.0

    tier: 0

    parameters:
        - test_metadata:
            type: data
            brief: Configuration cases.
        - set_guardsarm_configuration:
            type: fixture
            brief: Configure a custom environment for testing.
        - configure_local_internal_options:
            type: fixture
            brief: Set internal configuration for testing.
        - truncate_monitored_files:
            type: fixture
            brief: Reset the 'ossec.log' file and start a new monitor.
        - clean_keys:
            type: fixture
            brief: Cleans keys file content
        - add_keys:
            type: fixture
            brief: Adds keys to keys file
        - daemons_handler:
            type: fixture
            brief: Handler of GuardSarm daemons.

    assertions:
        - Verify that the agent enrollment is successful.

    input_description: An external YAML file (guardsarm_conf.yaml) includes configuration settings for the agent.
                       Two test cases are found in the test module and include parameters
                       for the environment setup using the TCP protocols.

    expected_output:
        - r'Valid key received'
        - r'Sending keep alive'

    tags:
        - simulator
        - ssl
        - keys
    '''

    try:
        # Start RemotedSimulator
        remoted_server = RemotedSimulator(protocol = 'tcp')
        remoted_server.start()

        # Wait until Agent is connected
        wait_connect()

        # Reset simulator
        remoted_server.destroy()

        # Start rejecting Agent
        remoted_server = RemotedSimulator(protocol = 'tcp', mode = 'WRONG_KEY')
        remoted_server.start()

        # Start AuthdSimulator
        authd_server = AuthdSimulator()
        authd_server.start()

        # Wait until Agent asks a new key to enrollment
        wait_enrollment_try()

        # Reset simulator
        remoted_server.destroy()

        # Start responding Agent
        remoted_server = RemotedSimulator(protocol = 'tcp')
        remoted_server.start()

        # Wait until Agent is connected
        wait_connect()
    finally:
        # Reset simulator
        remoted_server.destroy()
        authd_server.destroy()
