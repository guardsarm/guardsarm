# Copyright (C) 2026 GuardSarm, Inc.
# Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.

from guardsarm.core.exception import GuardSarmException, GuardSarmError
import pytest


@pytest.mark.parametrize('code, extra_message, extra_remediation, cmd_error, dapi_errors, title, type, exc_string', [
        # code not found in ERRORS - use extra_message
        (9999, "External exception", None, None, None, None, None, "Error 9999 - External exception"),
        # code found in ERRORS - cmd_error True
        (999, "Code found with cmd_error", None, True, None, None, None, "Error 999 - Code found with cmd_error"),
        # code found in ERRORS - dictionary entry of string type
        (999, None, None, None, None, None, None, "Error 999 - Incompatible version of Python"),
        # code found in ERRORS - dictionary entry of dictionary type - withouth remediation key
        (4018, None, None, None, None, None, None, "Error 4018 - Level cannot be a negative number"),
        # code found in ERRORS - dictionary entry of dictionary type - with remediation key
        (4019, None, None, None, None, None, None, "Error 4019 - Invalid resource specified"),
        # code found in ERRORS - extra_message parameter of string type
        (4018, "extra message", None, None, None, None, None, "Error 4018 - Level cannot be a negative number: extra message"),
        # code found in ERRORS - extra_message parameter of dictionary type
        (1017, {'node_name': 'Node Name', 'not_ready_daemons': 'not ready daemons'}, None, None, None, None, None, 
            'Error 1017 - Some GuardSarm daemons are not ready yet in node "Node Name" (not ready daemons)'),        
    ])
def test_guardsarm_exception_to_string(code, extra_message, extra_remediation, cmd_error, dapi_errors, title , type, exc_string):
    """Check object constructor """
    exc = GuardSarmException(code, extra_message, extra_remediation, cmd_error, dapi_errors, title, type)
    assert str(exc) == exc_string


def test_guardsarm_exception__or__():
    """Check that GuardSarmException's | operator performs the join of dapi errors properly."""
    excp1 = GuardSarmException(1307)
    excp1._dapi_errors = {'test1': 'test error'}
    excp2 = GuardSarmException(1307)
    excp2._dapi_errors = {'test2': 'test error'}
    excp3 = excp2 | excp1
    assert excp3._dapi_errors == {'test1': 'test error', 'test2': 'test error'}


def test_guardsarm_exception__deepcopy__():
    """Check that GuardSarmException's __deepcopy__ magic method works properly."""
    excp1 = GuardSarmException(1307)
    excp2 = excp1.__deepcopy__()
    assert excp1 == excp2 and excp1 is not excp2


def test_guardsarm_error__or__():
    """Check that GuardSarmError's | operator performs the union of id sets properly."""
    error1 = GuardSarmError(1000, ids={1, 2, 3})
    error2 = GuardSarmError(1000, ids={4, 5, 6})
    error3 = error2 | error1
    assert error3.ids == {1, 2, 3, 4, 5, 6}
