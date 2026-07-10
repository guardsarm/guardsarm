# Copyright (C) 2026 GuardSarm, Inc.
# Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
from pathlib import Path

# Constants & base paths
DATA_PATH = Path(Path(__file__).parent, 'data')
CONFIGS_PATH = Path(DATA_PATH, 'configuration_templates')
TEST_CASES_PATH = Path(DATA_PATH, 'test_cases')
