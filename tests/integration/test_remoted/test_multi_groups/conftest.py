"""
 Copyright (C) 2026 GuardSarm, Inc.
 Created by Wazuh, Inc. <info@wazuh.com>.
 Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
"""

import pytest
import time

from guardsarm_testing.tools.simulators import agent_simulator
from guardsarm_testing.utils.agent_groups import create_group, delete_group, add_agent_to_group


@pytest.fixture()
def prepare_environment(request, simulate_agents):
    """Configure a custom environment for testing."""
    agent = simulate_agents[0]
    agent_id = agent.id
    sender, injector = agent_simulator.connect(agent)

    create_group('testing_group')
    add_agent_to_group('testing_group', agent_id)
    time.sleep(20)

    yield
    delete_group('testing_group')
    injector.stop_receive()
