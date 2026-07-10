# Copyright (C) 2026 GuardSarm, Inc.
# Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.

import pytest

from guardsarm_testing.constants.paths.logs import GUARDSARM_LOG_PATH
from guardsarm_testing.modules.modulesd import patterns
from guardsarm_testing.tools.monitors.file_monitor import FileMonitor
from guardsarm_testing.utils import callbacks


@pytest.fixture()
def wait_for_office365_start():
    # Wait for module office365 starts
    guardsarm_log_monitor = FileMonitor(GUARDSARM_LOG_PATH)
    guardsarm_log_monitor.start(callback=callbacks.generate_callback(patterns.MODULESD_STARTED, {
                              'integration': 'Office365'
                          }))
    assert (guardsarm_log_monitor.callback_result == None), f'Error invalid configuration event not detected'
