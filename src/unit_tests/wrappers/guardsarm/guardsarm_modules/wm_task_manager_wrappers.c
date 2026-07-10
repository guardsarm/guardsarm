/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#include "../../common.h"
#include "wm_task_manager_wrappers.h"
#include <stddef.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>

cJSON* __wrap_wm_task_manager_parse_message(const char *msg) {
    check_expected(msg);

    return mock_type(cJSON*);
}

cJSON* __wrap_wm_task_manager_process_task(const gm_task_manager_task *task, int *error_code) {
    check_expected(task);

    *error_code = mock();

    return mock_type(cJSON*);
}

cJSON* __wrap_wm_task_manager_parse_data_response(int error_code, int agent_id, int task_id, char *status) {
    check_expected(error_code);
    check_expected(agent_id);
    check_expected(task_id);
    if (status) check_expected(status);

    return mock_type(cJSON*);
}

void __wrap_wm_task_manager_parse_data_result(__attribute__ ((__unused__)) cJSON *response, const char *node, const char *module, const char *command, char *status, char *error, int create_time, int last_update_time, char *request_command) {
    check_expected(node);
    check_expected(module);
    check_expected(command);
    check_expected(status);
    check_expected(error);
    check_expected(create_time);
    check_expected(last_update_time);
    check_expected(request_command);
}
