"""
Copyright (C) 2026 GuardSarm, Inc.
Created by Wazuh, Inc. <info@wazuh.com>.
Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
"""
import pytest

from guardsarm_testing.constants.paths.configurations import GUARDSARM_CLIENT_KEYS_PATH
from guardsarm_testing.utils import file
from . import utils


@pytest.fixture()
def clean_agents_ctx(stop_authd):
    """
    Clean agents files.
    """
    file.truncate_file(GUARDSARM_CLIENT_KEYS_PATH)
    utils.clean_rids()
    utils.clean_agents_timestamp()
    utils.clean_diff()

    yield

    file.truncate_file(GUARDSARM_CLIENT_KEYS_PATH)
    utils.clean_rids()
    utils.clean_agents_timestamp()
    utils.clean_diff()
