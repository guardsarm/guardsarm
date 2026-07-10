'''
copyright: Copyright (C) 2026 GuardSarm, Inc.
           Created by Wazuh, Inc. <info@wazuh.com>.
           Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
type: integration
brief: GuardSarm-db is the daemon in charge of the databases with all the GuardSarm persistent information, exposing a socket
       to receive requests and provide information. This test checks the guardsarmdb getconfig command used to get the
       current configuration.

tier: 0

modules:
    - guardsarm_db

components:
    - manager

daemons:
    - guardsarm-manager-db

os_platform:
    - linux

os_version:
    - Arch Linux
    - Amazon Linux 2
    - Amazon Linux 1
    - CentOS 8
    - CentOS 7
    - CentOS 6
    - Ubuntu Focal
    - Ubuntu Bionic
    - Ubuntu Xenial
    - Ubuntu Trusty
    - Debian Buster
    - Debian Stretch
    - Debian Jessie
    - Debian Wheezy
    - Red Hat 8
    - Red Hat 7
    - Red Hat 6

references:
    - https://documentation.guardsarmsiem.com/current/user-manual/reference/daemons/guardsarm-manager-db.html

tags:
    - guardsarm_db
'''
from pathlib import Path
import pytest
from guardsarm_testing.utils.database import query_wdb
from guardsarm_testing.utils import configuration

from . import TEST_CASES_FOLDER_PATH

# Marks
pytestmark = [pytest.mark.server, pytest.mark.tier(level=0)]

# Configurations
t_cases_path = Path(TEST_CASES_FOLDER_PATH, 'cases_guardsarmdb_getconfig.yaml')
t_config_parameters, t_config_metadata, t_case_ids = configuration.get_test_cases_data(t_cases_path)

# Test daemons to restart.
daemons_handler_configuration = {'all_daemons': True}


# Tests
@pytest.mark.parametrize('test_metadata', t_config_metadata, ids=t_case_ids)
def test_guardsarmdb_getconfig(test_metadata, daemons_handler_module):
    '''
    description: Check that commands about guardsarmdb getconfig works properly.
    guardsarm_min_version: 5.0.0
    parameters:
        - test_metadata:
            type: dict
            brief: Test case metadata.
        - daemons_handler_module:
            type: fixture
            brief: Handler of GuardSarm daemons.
    assertions:
        - Verify that the socket response matches the expected output.
    input_description:
        - Test cases are defined in the guardsarmdb_getconfig.yaml file.
    expected_output:
        - an array with the configuration of DB.
    tags:
        - guardsarm_db
        - wdb_socket
    '''
    # Set each case
    output = test_metadata["output"]

    response = query_wdb(test_metadata["input"])

    # Validate response
    assert output in str(response), f"The expected output: {output} was not found in response: {response}"
