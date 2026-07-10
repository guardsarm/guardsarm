"""
Copyright (C) 2026 GuardSarm, Inc.
April 1, 2022.

Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
"""

import logging
import pytest


def pytest_addoption(parser):
    parser.addoption(
        "--moduleName",
        action="store",
        default=None,
        type=str,
        help="run test for a specific module")


@pytest.fixture(scope="session")
def getModuleName(request):
    return request.config.getoption("--moduleName")
