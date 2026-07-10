/*
 * GuardSarm Module for Task management.
 * Copyright (C) 2026 GuardSarm, Inc.
 * July 13, 2020.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */
#ifndef GM_TASK_MANAGER_H
#define GM_TASK_MANAGER_H

#include "wm_task_general.h"

#define GM_TASK_MANAGER_LOGTAG ARGV0 ":" TASK_MANAGER_WM_NAME

#define GM_TASK_MAX_IN_PROGRESS_TIME 900 // 15 minutes
#define GM_TASK_CLEANUP_DB_SLEEP_TIME 86400 // A day
#define GM_TASK_DEFAULT_CLEANUP_TIME 604800 // A week

typedef struct _wm_task_manager {
    int enabled:1;
    int cleanup_time;
    int task_timeout;
} gm_task_manager;

typedef enum _error_code {
    GM_TASK_SUCCESS = 0,
    GM_TASK_INVALID_MESSAGE,
    GM_TASK_INVALID_COMMAND,
    GM_TASK_DATABASE_NO_TASK,
    GM_TASK_DATABASE_ERROR,
    GM_TASK_DATABASE_PARSE_ERROR,
    GM_TASK_DATABASE_REQUEST_ERROR,
    GM_TASK_UNKNOWN_ERROR
} error_code;

/**
 * Definition of upgrade parameters
 */
typedef struct _wm_task_manager_upgrade {
    char *node;
    char *module;
    int *agent_ids;
} gm_task_manager_upgrade;

/**
 * Definition of upgrade get status parameters
 */
typedef struct _wm_task_manager_upgrade_get_status {
    char *node;
    int *agent_ids;
} gm_task_manager_upgrade_get_status;

/**
 * Definition of upgrade update status parameters
 */
typedef struct _wm_task_manager_upgrade_update_status {
    char *node;
    int *agent_ids;
    char *status;
    char *error_msg;
} gm_task_manager_upgrade_update_status;

/**
 * Definition of upgrade result parameters
 */
typedef struct _wm_task_manager_upgrade_result {
    int *agent_ids;
} gm_task_manager_upgrade_result;

/**
 * Definition of upgrade cancel tasks parameters
 */
typedef struct _wm_task_manager_upgrade_cancel_tasks {
    char *node;
} gm_task_manager_upgrade_cancel_tasks;

/**
 * Definition of task structure
 */
typedef struct _wm_task_manager_task {
    command_list command;
    void *parameters;
} gm_task_manager_task;

extern const gm_context GM_TASK_MANAGER_CONTEXT;   // Context

// Parse XML configuration
int gm_task_manager_read(const OS_XML *xml, xml_node **nodes, gmodule *module);

/**
 * Do all the analysis of the incomming message and returns a response.
 * @param msg Incomming message from a connection.
 * @param response Response to be sent to the connection.
 * @return Size of the response string.
 * */
size_t gm_task_manager_dispatch(const char *msg, char **response) __attribute__((nonnull));

/**
 * Process a task and call appropiate command function.
 * @param task Task to be processed.
 * @param error_code Variable to store an error code if something is wrong.
 * @return JSON object with the response for this task.
 * */
cJSON* gm_task_manager_process_task(const gm_task_manager_task *task, int *error_code) __attribute__((nonnull));

/**
 * Set tasks status to TIMEOUT after they are IN PROGRESS for a long period of time.
 * Delete entries older than a configurable period of time from the tasks DB.
 * @param arg Module configuration.
 * */
void* gm_task_manager_clean_tasks(void *arg) __attribute__((nonnull));

#endif
