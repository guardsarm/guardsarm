/*
 * GuardSarm Module for Task management.
 * Copyright (C) 2026 GuardSarm, Inc.
 * July 13, 2020.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#include "wm_task_general.h"

const char *task_manager_json_keys[] = {
    // Request
    [GM_TASK_ORIGIN] = "origin",
    [GM_TASK_NAME] = "name",
    [GM_TASK_MODULE] = "module",
    [GM_TASK_COMMAND] = "command",
    [GM_TASK_PARAMETERS] = "parameters",
    [GM_TASK_AGENTS] = "agents",
    // Response
    [GM_TASK_ERROR] = "error",
    [GM_TASK_DATA] = "data",
    [GM_TASK_ERROR_MESSAGE] = "message",
    [GM_TASK_AGENT_ID] = "agent",
    [GM_TASK_TASK_ID] = "task_id",
    [GM_TASK_NODE] = "node",
    [GM_TASK_STATUS] = "status",
    [GM_TASK_ERROR_MSG] = "error_msg",
    [GM_TASK_CREATE_TIME] = "create_time",
    [GM_TASK_LAST_UPDATE_TIME] = "update_time",
    // Clean tasks request
    [GM_TASK_NOW] = "now",
    [GM_TASK_INTERVAL] = "interval",
    [GM_TASK_TIMESTAMP] = "timestamp"
};

const char *task_manager_commands_list[] = {
    [GM_TASK_UPGRADE] = "upgrade",
    [GM_TASK_UPGRADE_CUSTOM] = "upgrade_custom",
    [GM_TASK_UPGRADE_GET_STATUS] = "upgrade_get_status",
    [GM_TASK_UPGRADE_UPDATE_STATUS] = "upgrade_update_status",
    [GM_TASK_UPGRADE_RESULT] = "upgrade_result",
    [GM_TASK_UPGRADE_CANCEL_TASKS] = "upgrade_cancel_tasks",
    [GM_TASK_SET_TIMEOUT] = "set_timeout",
    [GM_TASK_DELETE_OLD] = "delete_old"
};

const char *task_manager_modules_list[] = {
    [GM_TASK_UPGRADE_MODULE] = "upgrade_module",
    [GM_TASK_API_MODULE] = "api"
};

const char *task_statuses[] = {
    [GM_TASK_PENDING] = GM_TASK_STATUS_PENDING,
    [GM_TASK_IN_PROGRESS] = GM_TASK_STATUS_IN_PROGRESS,
    [GM_TASK_DONE] = GM_TASK_STATUS_DONE,
    [GM_TASK_FAILED] = GM_TASK_STATUS_FAILED,
    [GM_TASK_CANCELLED] = GM_TASK_STATUS_CANCELLED,
    [GM_TASK_TIMEOUT] = GM_TASK_STATUS_TIMEOUT,
    [GM_TASK_LEGACY] = GM_TASK_STATUS_LEGACY
};
