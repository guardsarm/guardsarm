"""
Copyright (C) 2026 GuardSarm, Inc.
Created by Wazuh, Inc. <info@wazuh.com>.
Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
"""
from pathlib import Path


# Constants & base paths
TEST_DATA_PATH = Path(Path(__file__).parent, 'data')
TEST_CASES_PATH = Path(TEST_DATA_PATH, 'test_cases')
CONFIGURATIONS_PATH = Path(TEST_DATA_PATH, 'configuration_templates')
