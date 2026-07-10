# Copyright (C) 2026 GuardSarm, Inc.
# Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.

from unittest.mock import patch
import pytest


with patch('guardsarm.core.common.guardsarm_uid'):
    with patch('guardsarm.core.common.guardsarm_gid'):
        from guardsarm.core.task import *
        from guardsarm.core.utils import GuardSarmDBQuery


def test_guardsarm_db_query_task__init__():
    """Check if GuardSarmDBQueryTask is properly initialized."""
    with patch('guardsarm.core.utils.GuardSarmDBQuery.__init__') as mock_wdbq, \
            patch('guardsarm.core.utils.GuardSarmDBBackend.__init__', return_value=None) as mock_wdb_backend:
        GuardSarmDBQueryTask()
        mock_wdbq.assert_called_once()
        mock_wdb_backend.assert_called_with(query_format='task')


def test_guardsarm_db_query_task__final_query():
    """Check if GuardSarmDBQueryTask's method _final_query works properly."""
    with patch('guardsarm.core.utils.GuardSarmDBBackend.__init__', return_value=None), \
            patch('guardsarm.core.utils.GuardSarmDBQuery._default_query', return_value='test'):
        wdbq_task = GuardSarmDBQueryTask()
        assert wdbq_task._final_query() == f'test WHERE task_id IN (test) LIMIT :limit OFFSET :offset'


def test_guardsarm_db_query_task__format_data_into_dictionary():
    """Check that GuardSarmDBQueryTask's method _format_data_into_dictionary works properly."""
    data = [
        {'task_id': 1, 'agent_id': '002', 'node': 'worker1', 'module': 'upgrade_module', 'command': 'upgrade',
         'create_time': 1606466932, 'last_update_time': 1606466953, 'status': 'Legacy', 'error_message': None}
    ]

    with patch('guardsarm.core.utils.GuardSarmDBBackend.__init__', return_value=None), \
            patch('guardsarm.core.utils.GuardSarmDBQuery._default_query', return_value='test'):
        wdbq_task = GuardSarmDBQueryTask()

    wdbq_task._data = data
    result = wdbq_task._format_data_into_dictionary()

    assert result['items'][0]['task_id'] == 1
    assert result['items'][0]['agent_id'] == '002'
    assert result['items'][0]['node'] == 'worker1'
    assert result['items'][0]['module'] == 'upgrade_module'
    assert result['items'][0]['command'] == 'upgrade'
    assert result['items'][0]['create_time'] == '2020-11-27T08:48:52Z'
    assert result['items'][0]['last_update_time'] == '2020-11-27T08:49:13Z'
    assert result['items'][0]['status'] == 'Legacy'
    assert result['items'][0]['error_message'] == None


@pytest.mark.parametrize('field_name, field_filter, q_filter', [
    ('agent_list', 'field', {'value': '1', 'operator': 'LIKE'}),
    ('task_list', 'test', {'value': '1', 'operator': 'LIKE'}),
    ('test_field', 'test', {'value': '1', 'operator': 'LIKE'})
])
def test_guardsarm_db_query_task__process_filter(field_name, field_filter, q_filter):
    """Check that GuardSarmDBQueryTask's method _process_filter works properly."""
    with patch('guardsarm.core.utils.GuardSarmDBBackend.__init__', return_value=None), \
            patch('guardsarm.core.utils.GuardSarmDBQuery._default_query', return_value='test'):
        wdbq_task = GuardSarmDBQueryTask()

    with patch.object(GuardSarmDBQuery, '_process_filter') as mock_sup_proc:
        wdbq_task._process_filter(field_name, field_filter, q_filter)
        if field_name == 'agent_list':
            assert f'agent_id {q_filter["operator"]} (:{field_filter})' in wdbq_task.query
            assert wdbq_task.request[field_filter] == q_filter['value']
        elif field_name == 'task_list':
            assert f'task_id {q_filter["operator"]} (:{field_filter})' in wdbq_task.query
            assert wdbq_task.request[field_filter] == q_filter['value']
        else:
            mock_sup_proc.assert_called()
