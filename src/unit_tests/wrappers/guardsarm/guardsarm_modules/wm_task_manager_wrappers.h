/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef GM_TASK_MANAGER_WRAPPERS_H
#define GM_TASK_MANAGER_WRAPPERS_H

#include "shared.h"
#include "wmodules.h"

cJSON* __wrap_wm_task_manager_parse_message(const char* msg);

cJSON* __wrap_wm_task_manager_process_task(const gm_task_manager_task* task, int* error_code);

cJSON* __wrap_wm_task_manager_parse_data_response(int error_code, int agent_id, int task_id, char* status);

void __wrap_wm_task_manager_parse_data_result(cJSON* response,
                                              const char* node,
                                              const char* module,
                                              const char* command,
                                              char* status,
                                              char* error,
                                              int create_time,
                                              int last_update_time,
                                              char* request_command);

#endif
