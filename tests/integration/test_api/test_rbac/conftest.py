"""
Copyright (C) 2026 GuardSarm, Inc.
Created by Wazuh, Inc. <info@wazuh.com>.
Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
"""
import pytest

from guardsarm_testing.modules.api.utils import add_resources, relate_resources, remove_resources


@pytest.fixture
def set_security_resources(wait_for_api_start, test_metadata: dict) -> None:
    """Configure the security resources using the API and clean the added resources.

    Args:
        wait_for_api_start: Ensure the API is ready before issuing RBAC requests.
        test_metadata (dict): Test metadata.
    """
    remove_resources(test_metadata)
    test_metadata = add_resources(test_metadata)
    relate_resources(test_metadata)

    yield

    remove_resources(test_metadata)
