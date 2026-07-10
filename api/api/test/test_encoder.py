# Copyright (C) 2026 GuardSarm, Inc.
# Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.

import json
from unittest.mock import patch

import pytest

with patch('guardsarmsiem.common.guardsarm_uid'):
    with patch('guardsarmsiem.common.guardsarm_gid'):
        from api.encoder import prettify, dumps
        from guardsarm.core.results import GuardSarmResult


def custom_hook(dct):
    if 'key' in dct:
        return {'key': dct['key']}
    elif 'error' in dct:
        return GuardSarmResult.decode_json({'result': dct, 'str_priority': 'v2'})
    else:
        return dct


@pytest.mark.parametrize('o', [{'key': 'v1'},
                               GuardSarmResult({'k1': 'v1'}, str_priority='v2')
                               ]
                         )
def test_encoder_dumps(o):
    """Test dumps method from API encoder using GuardSarmAPIJSONEncoder."""
    encoded = dumps(o)
    decoded = json.loads(encoded, object_hook=custom_hook)
    assert decoded == o


def test_encoder_prettify():
    """Test prettify method from API encoder using GuardSarmAPIJSONEncoder."""
    assert prettify({'k1': 'v1'}) == '{\n   "k1": "v1"\n}'
