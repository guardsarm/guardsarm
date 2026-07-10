# Copyright (C) 2026 GuardSarm, Inc.
# Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.

from unittest.mock import patch
import pytest

from api.uri_parser import APIUriParser, LOWER_FIELDS


@pytest.mark.parametrize('query_parm',
                         LOWER_FIELDS)
def test_apiuriparser_resolve_params(query_parm):
    """Test Parameter Sanitization."""


    uri_parser = APIUriParser({}, {})
    with patch('connexion.uri_parsing.AbstractURIParser.resolve_params') as resolv_mock:
        def side_effect_resolve_param(params, _in):
            return params

        resolv_mock.side_effect = side_effect_resolve_param
        parm = uri_parser.resolve_params({query_parm: 'ValuE'}, 'query')
        resolv_mock.assert_called_once_with(parm, 'query')
