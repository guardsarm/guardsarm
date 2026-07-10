/*
 * GuardSarm Module for Agent Upgrading
 * Copyright (C) 2026 GuardSarm, Inc.
 * July 30, 2020.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef GM_AGENT_UPGRADE_MANAGER_H
#define GM_AGENT_UPGRADE_MANAGER_H

#include "wm_agent_upgrade.h"
#include "wm_task_general.h"

#define GM_UPGRADE_MINIMAL_VERSION_SUPPORT "v3.0.0"
#define GM_UPGRADE_NEW_LINUX_VERSION_REPOSITORY "v3.4.0"
#define GM_UPGRADE_5X_MINIMUM_VERSION "v5.0.0"
#define GM_UPGRADE_REQUIRED_INTERMEDIATE_VERSION "v4.14.0"
#define GM_UPGRADE_NEW_VERSION_STRUCTURE_REPOSITORY "v4.9.0"
#define GM_UPGRADE_NEW_UPGRADE_MECHANISM "v4.1.0"
#define GM_UPGRADE_WPK_DEFAULT_PATH "var/upgrade/"
#define GM_UPGRADE_WPK_DOWNLOAD_TIMEOUT 60000
#define GM_UPGRADE_WPK_DOWNLOAD_ATTEMPTS 5
#define GM_UPGRADE_WPK_OPEN_ATTEMPTS 10
#define GM_UPGRADE_MAX_RESPONSE_SIZE 1048576L
#define GM_AGENT_UPGRADE_START_WAIT_TIME 30
#define GM_UPGRADE_DEFAULT_REQUEST_TIMEOUT 20L

typedef enum _wm_upgrade_error_code {
    GM_UPGRADE_SUCCESS = 0,
    GM_UPGRADE_PARSING_ERROR,
    GM_UPGRADE_PARSING_REQUIRED_PARAMETER,
    GM_UPGRADE_TASK_CONFIGURATIONS,
    GM_UPGRADE_TASK_MANAGER_COMMUNICATION,
    GM_UPGRADE_TASK_MANAGER_FAILURE,
    GM_UPGRADE_GLOBAL_DB_FAILURE,
    GM_UPGRADE_INVALID_ACTION_FOR_MANAGER,
    GM_UPGRADE_AGENT_IS_NOT_ACTIVE,
    GM_UPGRADE_SYSTEM_NOT_SUPPORTED,
    GM_UPGRADE_UPGRADE_ALREADY_IN_PROGRESS,
    GM_UPGRADE_NOT_MINIMAL_VERSION_SUPPORTED,
    GM_UPGRADE_INTERMEDIATE_VERSION_REQUIRED,
    GM_UPGRADE_NEW_VERSION_LESS_OR_EQUAL_THAN_CURRENT,
    GM_UPGRADE_NEW_VERSION_GREATER_MASTER,
    GM_UPGRADE_URL_NOT_FOUND,
    GM_UPGRADE_WPK_VERSION_DOES_NOT_EXIST,
    GM_UPGRADE_WPK_FILE_DOES_NOT_EXIST,
    GM_UPGRADE_WPK_SHA1_DOES_NOT_MATCH,
    GM_UPGRADE_SEND_LOCK_RESTART_ERROR,
    GM_UPGRADE_SEND_OPEN_ERROR,
    GM_UPGRADE_SEND_WRITE_ERROR,
    GM_UPGRADE_SEND_CLOSE_ERROR,
    GM_UPGRADE_SEND_SHA1_ERROR,
    GM_UPGRADE_SEND_UPGRADE_ERROR,
    GM_UPGRADE_UPGRADE_ERROR,
    GM_UPGRADE_UPGRADE_ERROR_MISSING_PACKAGE,
    GM_UPGRADE_UNKNOWN_ERROR
} gm_upgrade_error_code;

typedef enum _wm_upgrade_command {
    GM_UPGRADE_UPGRADE = GM_TASK_UPGRADE,
    GM_UPGRADE_UPGRADE_CUSTOM = GM_TASK_UPGRADE_CUSTOM,
    GM_UPGRADE_AGENT_GET_STATUS = GM_TASK_UPGRADE_GET_STATUS,
    GM_UPGRADE_AGENT_UPDATE_STATUS = GM_TASK_UPGRADE_UPDATE_STATUS,
    GM_UPGRADE_RESULT = GM_TASK_UPGRADE_RESULT,
    GM_UPGRADE_CANCEL_TASKS = GM_TASK_UPGRADE_CANCEL_TASKS
} gm_upgrade_command;

/**
 * Definition of upgrade task to be run
 */
typedef struct _wm_upgrade_task {
    char *wpk_repository;        ///> url to a wpk_repository
    char *custom_version;        ///> upgrade to a custom version
    bool use_http;               ///> when enabled uses http instead of https to connect to repository
    bool force_upgrade;          ///> when enabled forces upgrade
    char *wpk_version;           ///> WPK version to install
    char *wpk_file;              ///> WPK file name
    char *wpk_sha1;              ///> WPK sha1 to validate
    char *package_type;          ///> package type to send (for Linux systems)
} gm_upgrade_task;

/**
 * Definition of upgrade custom task to be run
 */
typedef struct _wm_upgrade_custom_task {
    char *custom_file_path;      ///> sets a custom file path. Should be available in all worker nodes
    char *custom_installer;      ///> sets a custom installer script. Should be available in all worker nodes
} gm_upgrade_custom_task;

/**
 * Definition of an agent status update task
 */
typedef struct _wm_upgrade_agent_status_task {
    unsigned int error_code;
    char *message;
    char *status;
} gm_upgrade_agent_status_task;

/**
 * Definition of the structure that will represent a certain task
 */
typedef struct _wm_task_info {
    gm_upgrade_command command;  ///> command that has been requested
    void *task;                  ///> pointer to a task structure (depends on command)
} gm_task_info;

/**
 * Definition of the structure with the information of a certain agent
 */
typedef struct _wm_agent_info {
    int agent_id;                ///> agent_id of the agent
    char *platform;              ///> platform of the agent
    char *major_version;         ///> OS major version of the agent
    char *minor_version;         ///> OS minor version of the agent
    char *architecture;          ///> architecture of the agent
    char *guardsarm_version;         ///> guardsarm version of the agent
    char *connection_status;     ///> connection_status of the agent
    char *package_type;          ///> package type of the agent (DEB, RPM, etc.)
} gm_agent_info;

/**
 * Definition of the structure that will represent an agent executing a certain task
 */
typedef struct _wm_agent_task {
    gm_agent_info *agent_info;   ///> pointer to agent_info structure
    gm_task_info *task_info;     ///> pointer to task_info structure
} gm_agent_task;

extern const char* upgrade_error_codes[];

/**
 * Start main loop of the upgrade module for the manager
 * @param manager_configs Manager configuration parameters
 * @param enabled Wheter the module will allow or not upgrades
 * */
void gm_agent_upgrade_start_manager_module(const gm_manager_configs* manager_configs, const int enabled) __attribute__((nonnull));

/**
 * Process an upgrade_cancel_tasks command
 * */
void gm_agent_upgrade_cancel_pending_upgrades();

/**
 * Process an upgrade command. Create the task for each agent_id, dispatches to task manager and
 * then starts upgrading process.
 * @param agent_ids array with the list of agents id
 * @param task pointer to a wm_upgrade_task structure
 * @return string with the response
 * */
char* gm_agent_upgrade_process_upgrade_command(const int* agent_ids, gm_upgrade_task* task) __attribute__((nonnull));

/**
 * Process an upgrade custom command. Create the task for each agent_id, dispatches to task manager and
 * then starts upgrading process.
 * @param agent_ids array with the list of agents id
 * @param task pointer to a wm_upgrade_custom_task structure
 * @return string with the response
 * */
char* gm_agent_upgrade_process_upgrade_custom_command(const int* agent_ids, gm_upgrade_custom_task* task) __attribute__((nonnull));

/**
 * Process an agent_upgraded command
 * @param agent_ids List with id of the agents (In this case the list will contain only 1 id)
 * @param task Task with the update information
 * @return string with the response
 * */
char* gm_agent_upgrade_process_agent_result_command(const int* agent_ids, const gm_upgrade_agent_status_task* task) __attribute__((nonnull));

/**
 * Process an upgrade_result command
 * @param agent_ids List with id of the agents (In this case the list will contain only 1 id)
 * @return string with the response
 * */
char* gm_agent_upgrade_process_upgrade_result_command(const int* agent_ids) __attribute__((nonnull));

#endif
