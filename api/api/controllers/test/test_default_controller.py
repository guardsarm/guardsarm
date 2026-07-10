# Copyright (C) 2026 GuardSarm, Inc.
# Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.

import sys
from unittest.mock import MagicMock, patch

import pytest
from connexion.lifecycle import ConnexionResponse

with patch('guardsarmsiem.common.guardsarm_uid'):
    with patch('guardsarmsiem.common.guardsarm_gid'):
        sys.modules['guardsarm.rbac.orm'] = MagicMock()
        import guardsarm.rbac.decorators
        from api.controllers.default_controller import (BasicInfo, DATE_FORMAT,
                                                        default_info, socket)
        from guardsarm.tests.util import RBAC_bypasser
        from guardsarm.core.utils import get_utc_now
        guardsarm.rbac.decorators.expose_resources = RBAC_bypasser
        del sys.modules['guardsarm.rbac.orm']


@pytest.mark.asyncio
@patch('api.controllers.default_controller.load_spec', return_value=MagicMock())
@patch('api.controllers.default_controller.GuardSarmResult', return_value={})
async def test_default_info(mock_wresult, mock_lspec):
    """Verify 'default_info' endpoint is working as expected."""
    result = await default_info()
    data = {
        'title': mock_lspec.return_value['info']['title'],
        'api_version': mock_lspec.return_value['info']['version'],
        'revision': mock_lspec.return_value['info']['x-revision'],
        'license_name': mock_lspec.return_value['info']['license']['name'],
        'license_url': mock_lspec.return_value['info']['license']['url'],
        'hostname': socket.gethostname(),
        'timestamp': get_utc_now().strftime(DATE_FORMAT)
    }
    mock_lspec.assert_called_once_with()
    mock_wresult.assert_called_once_with({'data': BasicInfo.from_dict(data)})
    assert isinstance(result, ConnexionResponse)
