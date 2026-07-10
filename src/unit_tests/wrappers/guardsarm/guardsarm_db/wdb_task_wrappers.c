/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#include <stddef.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>
#include "wdb_task_wrappers.h"

int __wrap_wdb_task_insert_task(__attribute__((unused)) wdb_t* wdb, int agent_id, const char *node, const char *module, const char *command) {
    check_expected(agent_id);
    check_expected(node);
    check_expected(module);
    check_expected(command);

    return mock();
}

int __wrap_wdb_task_get_upgrade_task_status(__attribute__((unused)) wdb_t* wdb, int agent_id, const char *node, char **status) {
    check_expected(agent_id);
    check_expected(node);

    os_strdup(mock_type(char*), *status);

    return mock();
}

int __wrap_wdb_task_update_upgrade_task_status(__attribute__((unused)) wdb_t* wdb, int agent_id, const char *node, const char *status, const char *error) {
    check_expected(agent_id);
    check_expected(node);
    if (status) check_expected(status);
    if (error) check_expected(error);

    return mock();
}

int __wrap_wdb_task_get_upgrade_task_by_agent_id(__attribute__((unused)) wdb_t* wdb, int agent_id, char **node, char **module, char **command, char **status, char **error, int *create_time, int *last_update_time) {
    check_expected(agent_id);

    os_strdup(mock_type(char*), *node);
    os_strdup(mock_type(char*), *module);
    os_strdup(mock_type(char*), *command);
    os_strdup(mock_type(char*), *status);
    os_strdup(mock_type(char*), *error);
    *create_time = mock();
    *last_update_time = mock();

    return mock();
}

int __wrap_wdb_task_cancel_upgrade_tasks(__attribute__((unused)) wdb_t* wdb, const char *node) {
    check_expected(node);

    return mock();
}

int __wrap_wdb_task_set_timeout_status(__attribute__((unused)) wdb_t* wdb, time_t now, int interval, time_t *next_timeout) {
    check_expected(now);
    check_expected(interval);

    *next_timeout = mock();

    return mock();
}

int __wrap_wdb_task_delete_old_entries(__attribute__((unused)) wdb_t* wdb, int timestamp) {
    check_expected(timestamp);

    return mock();
}

wdb_t* __wrap_wdb_open_tasks() {
    return mock_ptr_type(wdb_t*);
}
