"""
Copyright (C) 2026 GuardSarm, Inc.
Created by Wazuh, Inc. <info@wazuh.com>.
Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
"""
import pytest

from guardsarm_testing.constants.ports import DEFAULT_SSL_REMOTE_ENROLLMENT_PORT


@pytest.fixture()
def configure_receiver_sockets(request, test_metadata):
    """
    Get configurations from the module and set receiver sockets.
    """
    if test_metadata['ipv6'] == 'yes':
        receiver_sockets_params = [(("localhost", DEFAULT_SSL_REMOTE_ENROLLMENT_PORT), 'AF_INET6', 'SSL_TLSv1_2')]
    else:
        receiver_sockets_params = [(("localhost", DEFAULT_SSL_REMOTE_ENROLLMENT_PORT), 'AF_INET', 'SSL_TLSv1_2')]

    setattr(request.module, 'receiver_sockets_params', receiver_sockets_params)
