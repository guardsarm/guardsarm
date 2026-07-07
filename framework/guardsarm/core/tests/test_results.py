#!/usr/bin/env python
# Copyright (C) 2015, Wazuh Inc.
# Copyright (C) 2026, GuardSarm.
# Created by Wazuh, Inc. <info@wazuh.com>.
# This program is free software; you can redistribute it and/or modify it under the terms of GPLv2

from copy import deepcopy
from unittest.mock import patch

import pytest

with patch('guardsarm.core.common.guardsarm_uid'):
    with patch('guardsarm.core.common.guardsarm_gid'):
        from guardsarm.core.results import GuardSarmResult, AffectedItemsGuardSarmResult, _goes_before_than, nested_itemgetter, merge
        from guardsarm import GuardSarmException, GuardSarmError

param_name = ['affected_items', 'total_affected_items', 'sort_fields', 'sort_casting', 'sort_ascending',
              'all_msg', 'some_msg', 'none_msg']
GUARDSARM_EXCEPTION_CODE = 1725
FAILED_AGENT_ID = '999'


@pytest.fixture(scope='function')
def get_guardsarm_result():
    return GuardSarmResult(dct={"data": {"items": [{"item1": "data1"}, {"item2": "OK"}], "message": "Everything ok"}},
                       str_priority=['KO', 'OK'])


@pytest.fixture(scope='function')
def get_guardsarm_affected_item():
    def _get_affected(params=None):
        kwargs = {p_name: param for p_name, param in zip(param_name, params)}
        return AffectedItemsGuardSarmResult(**kwargs)

    return _get_affected


@pytest.fixture(scope='function')
def get_guardsarm_failed_item():
    item = AffectedItemsGuardSarmResult()
    item.add_failed_item(id_=FAILED_AGENT_ID, error=GuardSarmException(GUARDSARM_EXCEPTION_CODE))
    return item


@pytest.mark.parametrize('dikt, priority', [
    ({"data": {"items": [{"item1": "data1"}, {"item2": "OK"}], "message": "Everything ok"}}, ['KO', 'OK']),
    ({"data": {"items": [{"item1": "data1"}, {"item2": "data2"}], "message": "Everything ok"}}, None),
])
def test_results_GuardSarmResult__merge_str(dikt, priority, get_guardsarm_affected_item):
    """Test method `_merge_str` from `GuardSarmResult`.

        Parameters
        ----------
        dikt : dict
            Dict with basic information for the class declaration.
        priority : list
            Used to set the GuardSarmResult priority.
        """
    guardsarm_result = GuardSarmResult(deepcopy(dikt), str_priority=priority)
    assert isinstance(guardsarm_result, GuardSarmResult)
    item2 = guardsarm_result.dikt['data']['items'][1]['item2']
    merge_result = guardsarm_result._merge_str(item2, 'KO')
    assert merge_result == priority[0] if priority else '{}|{}'.format(item2, 'KO')


def test_results_GuardSarmResult_to_dict(get_guardsarm_result):
    """Test method `to_dict` from `GuardSarmResult`."""
    dict_result = get_guardsarm_result.to_dict()
    assert isinstance(dict_result, dict)
    assert (key == result_key for key, result_key in zip(['str_priority', 'result'], dict_result.keys()))


def test_results_GuardSarmResult_render(get_guardsarm_result):
    """Test method `render` from `GuardSarmResult`."""
    render_result = get_guardsarm_result.render()
    assert isinstance(render_result, dict)
    assert render_result == get_guardsarm_result.dikt


def test_results_GuardSarmResult_decode_json(get_guardsarm_result):
    """Test class method `decode_json` from `GuardSarmResult`."""
    guardsarm_result = get_guardsarm_result
    decoded_result = GuardSarmResult.decode_json(guardsarm_result.to_dict())
    assert decoded_result == guardsarm_result


@pytest.mark.parametrize('param_value', [
    # affected_items,total_affected_items, sort_fields, sort_casting, sort_ascending,
    # all_msg, some_msg, none_msg
    [['001', '002'], 2, None, ['int'], [True, True], 'Sample message', 'Sample message', 'Sample message'],
    [['001', '003'], None, None, ['int'], [True, False], 'Sample message', 'Sample message', 'Sample message'],
    [[], 0, None, None, ['int'], None, 'Sample message', 'Sample message', 'Sample message'],
    [['001'], None, None, ['str'], None, 'Sample message', 'Sample message', 'Sample message']
])
def test_results_AffectedItemsGuardSarmResult(get_guardsarm_affected_item, param_value):
    """Test class `AffectedItemsGuardSarmResult`.

    Parameters
    ----------
    param_value : list
        List with param values for _init_.
    """
    affected_result = get_guardsarm_affected_item(param_value)
    assert isinstance(affected_result, AffectedItemsGuardSarmResult)
    for value, dikt_value in zip(param_value, affected_result.dikt.values()):
        assert value == dikt_value


def test_results_AffectedItemsGuardSarmResult_add_failed_item():
    """Test method `add_failed_item` from class `AffectedItemsGuardSarmResult`."""
    affected_result = AffectedItemsGuardSarmResult()
    id_list = ['001', '002']
    # Add two failed items with different id but same exception
    for agent_id in id_list:
        affected_result.add_failed_item(id_=agent_id, error=GuardSarmException(GUARDSARM_EXCEPTION_CODE))

    assert affected_result.failed_items
    assert set(id_list) == next(iter(affected_result.failed_items.values()))


def test_results_AffectedItemsGuardSarmResult_add_failed_items_from(get_guardsarm_failed_item):
    """Test method `add_failed_items_from` from class `AffectedItemsGuardSarmResult`."""
    affected_result = AffectedItemsGuardSarmResult()
    failed_result = get_guardsarm_failed_item
    affected_result.add_failed_items_from(failed_result)
    assert affected_result.failed_items == failed_result.failed_items


def test_results_AffectedItemsGuardSarmResult_add_failed_items_from_exception():
    """Test raised exception from method `add_failed_items_from` from class `AffectedItemsGuardSarmResult`."""
    affected_result = AffectedItemsGuardSarmResult()
    with pytest.raises(GuardSarmException, match='.* 1000 .*'):
        affected_result.add_failed_items_from('Invalid type')


def test_results_AffectedItemsGuardSarmResult_remove_failed_items(get_guardsarm_failed_item):
    """Test method `remove_failed_items` from class `AffectedItemsGuardSarmResult`."""
    failed_result = get_guardsarm_failed_item
    failed_result.remove_failed_items(code={GUARDSARM_EXCEPTION_CODE})
    assert not failed_result.failed_items


def test_results_AffectedItemsGuardSarmResult___or__(get_guardsarm_failed_item):
    """Test method `__or__` from class `AffectedItemsGuardSarmResult`."""
    agent_list_1 = ['001', '002']
    agent_list_2 = ['004', '003']
    affected_item_1 = AffectedItemsGuardSarmResult(affected_items=deepcopy(agent_list_1))
    affected_item_2 = AffectedItemsGuardSarmResult(affected_items=deepcopy(agent_list_2))
    failed_item = get_guardsarm_failed_item

    # Expect 'affected_items': ['001', '002', '003']
    or_result_1 = affected_item_1 | affected_item_2
    assert set(agent_list_1 + agent_list_2) == set(or_result_1.affected_items)
    assert not or_result_1.failed_items

    # Expect new failed_item
    or_result_2 = or_result_1 | failed_item
    assert or_result_2.failed_items == failed_item.failed_items


@pytest.mark.parametrize('or_item, expected_result', [
    (GuardSarmError(GUARDSARM_EXCEPTION_CODE, ids=['001']), AffectedItemsGuardSarmResult),
    (GuardSarmError(GUARDSARM_EXCEPTION_CODE), GuardSarmException),
    (GuardSarmException(GUARDSARM_EXCEPTION_CODE), GuardSarmException),
    ({'Invalid type': None}, None)
])
def test_results_AffectedItemsGuardSarmResult___or___exceptions(or_item, expected_result):
    """Test raised exceptions from method `__or__` from class `AffectedItemsGuardSarmResult`."""
    affected_result = AffectedItemsGuardSarmResult()
    # Force an exception trying to use __or__ with an invalid type
    try:
        or_result = affected_result | or_item
        assert isinstance(or_result, expected_result)
    except GuardSarmException as e:
        if e.code != 1000:
            raise e


def test_results_AffectedItemsGuardSarmResult_to_dict():
    """Test method `to_dict` from class `AffectedItemsGuardSarmResult`."""
    affected_result = AffectedItemsGuardSarmResult()
    dict_item = affected_result.to_dict()
    assert isinstance(dict_item, dict)
    assert dict_item
    assert (field == dict_field for field, dict_field in zip(affected_result, dict_item))


def test_results_AffectedItemsGuardSarmResult_properties():
    """Test getters and setters from class `AffectedItemsGuardSarmResult`."""
    affected_result = AffectedItemsGuardSarmResult()
    # Lacks 'failed_items', 'total_failed_items' and 'message'
    property_list = ['affected_items', 'sort_fields', 'sort_casting', 'sort_ascending', 'total_affected_items',
                     'all_msg', 'some_msg', 'none_msg']
    values_list = [['001', '002'], 2, param_name, ['int'], [True, True], 'Sample message', 'Sample message',
                   'Sample message']

    assert len(property_list) == len(values_list)
    # Check getters and setters dynamically
    for key, value in zip(property_list, values_list):
        setattr(affected_result, key, value)
        assert value == getattr(affected_result, key)


def test_results_AffectedItemsGuardSarmResult_failed_items_properties(get_guardsarm_failed_item):
    """Test `failed_items` properties from class `AffectedItemsGuardSarmResult`."""
    fail_items = get_guardsarm_failed_item.failed_items
    total_fail_items = get_guardsarm_failed_item.total_failed_items
    assert isinstance(fail_items, dict)
    assert total_fail_items == 1


def test_results_AffectedItemsGuardSarmResult_message_property():
    """Test `message` property from class `AffectedItemsGuardSarmResult`."""
    messages = {'none_msg': 'none_msg', 'all_msg': 'all_msg', 'some_msg': 'some_msg'}
    # Force every possible case since this property returns a different value depending on affected_items
    none_msg_result = AffectedItemsGuardSarmResult(**messages).message
    all_msg_result = AffectedItemsGuardSarmResult(**messages, affected_items=['001']).message
    aux_result = AffectedItemsGuardSarmResult(**messages, affected_items=['001'])
    aux_result.add_failed_item(GuardSarmException(GUARDSARM_EXCEPTION_CODE))
    some_msg_result = aux_result.message
    assert messages
    assert len(messages) == 3
    assert (msg == item_msg for msg, item_msg in zip(messages, [none_msg_result, all_msg_result, some_msg_result]))


@pytest.mark.parametrize('self_field, other_field, key, expected_result', [
    ('Sample1', 'Sample2', 'older_than', 'Sample1'),
    ('Sample1', 'Sample2', None, 'Sample1|Sample2')
])
def test_results_AffectedItemsGuardSarmResult__merge_str(self_field, other_field, key, expected_result):
    """Test method `_merge_str` from class `AffectedItemsGuardSarmResult`."""
    affected_result = AffectedItemsGuardSarmResult()
    merge_result = affected_result._merge_str(self_field, other_field, key=key)
    assert merge_result == expected_result


def test_results_AffectedItemsGuardSarmResult_encode_decode_json(get_guardsarm_affected_item):
    """Test methods `encode_json` and `decode_json` from class `AffectedItemsGuardSarmResult`."""
    param_list = [['001', '002'], 2, None, ['int'], [True, True], 'Sample message', 'Sample message', 'Sample message']
    affected_result = get_guardsarm_affected_item(param_list)
    affected_result.add_failed_item(id_=FAILED_AGENT_ID, error=GuardSarmException(GUARDSARM_EXCEPTION_CODE))
    # Use a complete AffectedIemsGuardSarmResult to encode a json and then decode it
    json_item = affected_result.encode_json()
    decoded_json = AffectedItemsGuardSarmResult.decode_json(json_item)
    assert affected_result == decoded_json


def test_results_AffectedItemsGuardSarmResult_render(get_guardsarm_affected_item):
    """Test method `render` from class `AffectedItemsGuardSarmResult`."""
    param_list = [['001', '002'], 2, None, ['int'], [True, True], 'Sample message', 'Sample message', 'Sample message']
    affected_result = get_guardsarm_affected_item(param_list)
    for agent_id in [FAILED_AGENT_ID, 'Invalid ID']:
        affected_result.add_failed_item(id_=agent_id, error=GuardSarmException(GUARDSARM_EXCEPTION_CODE))
    # Render a valid AffectedItemsGuardSarmResult and check it has all the expected fields
    render_result = affected_result.render()
    assert isinstance(render_result, dict)
    assert render_result
    assert (field in ['data', 'message'] for field in render_result)
    assert render_result['data']
    assert (field in ['affected_items', 'total_affected_items', 'total_failed_items', 'failed_items']
            for field in render_result['data'])


@pytest.mark.parametrize('item, expressions, expected_result', [
    ({'a': {'b': 3}, 'c.1': 5}, ['a.b', 'c\\.1'], (3, 5)),
    ({'a': {'b': 3}, 'c.1': 5}, ['a.b', 'f'], (3, None)),
    ([{'a': {'b': 3}, 'c.1': 5}], ['c\\.1'], [{'a': {'b': 3}, 'c.1': 5}])
])
def test_results_nested_itemgetter(item, expressions, expected_result):
    """Test function `nested_itemgetter` from module results

    Parameters
    ----------
    item : dict or list
        Dict to get data from. We use a list to force a TypeError.
    expressions : list(str)
        Expressions used to find certain data from `item`.
    expected_result : tuple
        Tuple with the expected result to assert if the function is working properly.
    """
    assert expected_result == nested_itemgetter(*expressions)(item)


@pytest.mark.parametrize('a, b, ascending, casters, expected_result', [
    (['sample'], ['elpmas'], None, None, False),
    (['sample'], ['elpmas'], [False], [list], True),
    (['sample'], ['elpmas'], [True], [str], False),
    (['elpmas'], ['sample'], [True], [None], True),
    ([None], [None], [True], [None], False),
    ([None], ['sample'], [True], [None], True),
    (['sample'], [None], [True], [None], False),
    (['equal'], ['equal'], None, [str], False)
])
def test_results__goes_before_than(a, b, ascending, casters, expected_result):
    """Test function `_goes_before_than` from module results.

    Parameters
    ----------
    a : tuple or list
        Tuple or list to be compared.
    b : tuple or list
        Tuple or list to be compared.
    ascending : list(bool)
        Tuple or list of booleans with a length equal to the minimum length between `a` and `b`. True if ascending,
        False otherwise.
    casters : iterable
        Iterable of callables with a length equal to the minimum length between `a` and `b`. The callable msut fit any
        class in builtins module.
    expected_result : bool
        Expected result after the method call.
    """
    assert _goes_before_than(a, b, ascending=ascending, casters=casters) == expected_result


@pytest.mark.parametrize('iterables, criteria, ascending, types, expected_result', [
    ((['001', '002'], ['003', '004']), None, [True], ['int'], ['001', '002', '003', '004']),
    ((['001', '002'], ['003', '004']), None, [False], ['int'], ['003', '004', '001', '002']),
    ((['001', '002'], ['003', '004']), ['1'], [True], ['int'], ['001', '002', '003', '004']),
])
def test_results_merge(iterables, criteria, ascending, types, expected_result):
    """Test function `merge` from module results.

    Parameters
    ----------
    iterables : list(list) or tuple(list)
        List of lists to be merged.
    criteria : list(str) or tuple(str)
        Expressions accepted by the `nested_itemgetter` function.
    ascending : list(bool) or tuple(bool)
        True for ascending, False otherwise.
    types : list(str) or tuple(str)
        Must fit a class in builtins.
    expected_result : list(str)
        Expected results after merge.
    """
    assert merge(*iterables, criteria=criteria, ascending=ascending, types=types) == expected_result
