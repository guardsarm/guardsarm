/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef WDB_TASK_WRAPPERS_H
#define WDB_TASK_WRAPPERS_H

#include "wdb.h"

int __wrap_wdb_task_insert_task(
    __attribute__((unused)) wdb_t* wdb, int agent_id, const char* node, const char* module, const char* command);
int __wrap_wdb_task_get_upgrade_task_status(__attribute__((unused)) wdb_t* wdb,
                                            int agent_id,
                                            const char* node,
                                            char** status);
int __wrap_wdb_task_update_upgrade_task_status(
    __attribute__((unused)) wdb_t* wdb, int agent_id, const char* node, const char* status, const char* error);
int __wrap_wdb_task_get_upgrade_task_by_agent_id(__attribute__((unused)) wdb_t* wdb,
                                                 int agent_id,
                                                 char** node,
                                                 char** module,
                                                 char** command,
                                                 char** status,
                                                 char** error,
                                                 int* create_time,
                                                 int* last_update_time);
int __wrap_wdb_task_cancel_upgrade_tasks(__attribute__((unused)) wdb_t* wdb, const char* node);
int __wrap_wdb_task_set_timeout_status(__attribute__((unused)) wdb_t* wdb,
                                       time_t now,
                                       int interval,
                                       time_t* next_timeout);
int __wrap_wdb_task_delete_old_entries(__attribute__((unused)) wdb_t* wdb, int timestamp);

#endif
