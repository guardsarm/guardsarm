/*
 * GuardSarm Module for Agent Upgrading
 * Copyright (C) 2026 GuardSarm, Inc.
 * October 19, 2020.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifdef GUARDSARM_UNIT_TESTING
// Remove STATIC qualifier from tests
#define STATIC
#else
#define STATIC static
#endif

#include "wmodules.h"
#include "wm_task_manager_tasks.h"

gm_task_manager_upgrade* gm_task_manager_init_upgrade_parameters() {
    gm_task_manager_upgrade *parameters;
    os_calloc(1, sizeof(gm_task_manager_upgrade), parameters);
    return parameters;
}

gm_task_manager_upgrade_get_status* gm_task_manager_init_upgrade_get_status_parameters() {
    gm_task_manager_upgrade_get_status *parameters;
    os_calloc(1, sizeof(gm_task_manager_upgrade_get_status), parameters);
    return parameters;
}

gm_task_manager_upgrade_update_status* gm_task_manager_init_upgrade_update_status_parameters() {
    gm_task_manager_upgrade_update_status *parameters;
    os_calloc(1, sizeof(gm_task_manager_upgrade_update_status), parameters);
    return parameters;
}

gm_task_manager_upgrade_result* gm_task_manager_init_upgrade_result_parameters() {
    gm_task_manager_upgrade_result *parameters;
    os_calloc(1, sizeof(gm_task_manager_upgrade_result), parameters);
    return parameters;
}

gm_task_manager_upgrade_cancel_tasks* gm_task_manager_init_upgrade_cancel_tasks_parameters() {
    gm_task_manager_upgrade_cancel_tasks *parameters;
    os_calloc(1, sizeof(gm_task_manager_upgrade_cancel_tasks), parameters);
    return parameters;
}

gm_task_manager_task* gm_task_manager_init_task() {
    gm_task_manager_task *task;
    os_calloc(1, sizeof(gm_task_manager_task), task);
    return task;
}

void gm_task_manager_free_upgrade_parameters(gm_task_manager_upgrade* parameters) {
    if (parameters) {
        os_free(parameters->node);
        os_free(parameters->module);
        os_free(parameters->agent_ids);
        os_free(parameters);
    }
}

void gm_task_manager_free_upgrade_get_status_parameters(gm_task_manager_upgrade_get_status* parameters) {
    if (parameters) {
        os_free(parameters->node);
        os_free(parameters->agent_ids);
        os_free(parameters);
    }
}

void gm_task_manager_free_upgrade_update_status_parameters(gm_task_manager_upgrade_update_status* parameters) {
    if (parameters) {
        os_free(parameters->node);
        os_free(parameters->agent_ids);
        os_free(parameters->status);
        os_free(parameters->error_msg);
        os_free(parameters);
    }
}

void gm_task_manager_free_upgrade_result_parameters(gm_task_manager_upgrade_result* parameters) {
    if (parameters) {
        os_free(parameters->agent_ids);
        os_free(parameters);
    }
}

void gm_task_manager_free_upgrade_cancel_tasks_parameters(gm_task_manager_upgrade_cancel_tasks* parameters) {
    if (parameters) {
        os_free(parameters->node);
        os_free(parameters);
    }
}

void gm_task_manager_free_task(gm_task_manager_task* task) {
    if (task) {
        if (task->parameters) {
            if ((GM_TASK_UPGRADE == task->command) || (GM_TASK_UPGRADE_CUSTOM == task->command)) {
                gm_task_manager_free_upgrade_parameters((gm_task_manager_upgrade*)task->parameters);
            } else if (GM_TASK_UPGRADE_GET_STATUS == task->command) {
                gm_task_manager_free_upgrade_get_status_parameters((gm_task_manager_upgrade_get_status*)task->parameters);
            } else if (GM_TASK_UPGRADE_UPDATE_STATUS == task->command) {
                gm_task_manager_free_upgrade_update_status_parameters((gm_task_manager_upgrade_update_status*)task->parameters);
            } else if (GM_TASK_UPGRADE_RESULT == task->command) {
                gm_task_manager_free_upgrade_result_parameters((gm_task_manager_upgrade_result*)task->parameters);
            } else if (GM_TASK_UPGRADE_CANCEL_TASKS == task->command) {
                gm_task_manager_free_upgrade_cancel_tasks_parameters((gm_task_manager_upgrade_cancel_tasks*)task->parameters);
            }
        }
        os_free(task);
    }
}
