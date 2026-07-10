/*
 * GuardSarm Module for Task management.
 * Copyright (C) 2026 GuardSarm, Inc.
 * July 13, 2020.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef GM_TASK_GENERAL_H
#define GM_TASK_GENERAL_H

#define GM_TASK_STATUS_PENDING "Pending"
#define GM_TASK_STATUS_IN_PROGRESS "In progress"
#define GM_TASK_STATUS_DONE "Done"
#define GM_TASK_STATUS_FAILED "Failed"
#define GM_TASK_STATUS_CANCELLED "Cancelled"
#define GM_TASK_STATUS_TIMEOUT "Timeout"
#define GM_TASK_STATUS_LEGACY "Legacy"

/**
 * Enumeration of all available keys that could be used in the messages
 * */
typedef enum _task_manager_json_key {
    // Request
    GM_TASK_ORIGIN = 0,
    GM_TASK_NAME,
    GM_TASK_MODULE,
    GM_TASK_COMMAND,
    GM_TASK_PARAMETERS,
    GM_TASK_AGENTS,
    // Response
    GM_TASK_ERROR,
    GM_TASK_DATA,
    GM_TASK_ERROR_MESSAGE,
    GM_TASK_AGENT_ID,
    GM_TASK_TASK_ID,
    GM_TASK_NODE,
    GM_TASK_STATUS,
    GM_TASK_ERROR_MSG,
    GM_TASK_CREATE_TIME,
    GM_TASK_LAST_UPDATE_TIME,
    // Clean tasks request
    GM_TASK_NOW,
    GM_TASK_INTERVAL,
    GM_TASK_TIMESTAMP
} task_manager_json_key;

/**
 * Enumeration of the available commands
 * */
typedef enum _command_list {
    GM_TASK_UPGRADE = 0,
    GM_TASK_UPGRADE_CUSTOM,
    GM_TASK_UPGRADE_GET_STATUS,
    GM_TASK_UPGRADE_UPDATE_STATUS,
    GM_TASK_UPGRADE_RESULT,
    GM_TASK_UPGRADE_CANCEL_TASKS,
    GM_TASK_SET_TIMEOUT,
    GM_TASK_DELETE_OLD,
    GM_TASK_UNKNOWN
} command_list;

/**
 * Enumeration of the modules orchestrated by the task manager
 * */
typedef enum _module_list {
    GM_TASK_UPGRADE_MODULE = 0,
    GM_TASK_API_MODULE
} module_list;

/**
 * Enumeration of the possible task statuses
 * */
typedef enum _task_status {
    GM_TASK_PENDING = 0,
    GM_TASK_IN_PROGRESS,
    GM_TASK_DONE,
    GM_TASK_FAILED,
    GM_TASK_CANCELLED,
    GM_TASK_TIMEOUT,
    GM_TASK_LEGACY
} task_status;

/**
 * List containing all the possible json_keys
 * */
extern const char *task_manager_json_keys[];

/**
 * List containing all the command names
 * */
extern const char *task_manager_commands_list[];

/**
 * List containing the module names
 * */
extern const char *task_manager_modules_list[];

/**
 * List of possible task statuses
 * */
extern const char *task_statuses[];

#endif
