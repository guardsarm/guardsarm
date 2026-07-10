/*
 * GuardSarm Module for Agent Upgrading
 * Copyright (C) 2026 GuardSarm, Inc.
 * August 10, 2020.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */
#ifndef GM_AGENT_UPGRADE_VALIDATE_H
#define GM_AGENT_UPGRADE_VALIDATE_H

#include "wm_agent_upgrade_manager.h"

/**
 * Check if agent exist
 * @param agent_id id of agent to validate
 * @return return_code
 * @retval WM_UPGRADE_SUCCESS
 * @retval WM_UPGRADE_UPGRADE_ERROR
 * */
int gm_agent_upgrade_validate_id(int agent_id);

/**
 * Check if agent status is active
 * @param connection_status connection status of the agent to validate
 * @return return_code
 * @retval WM_UPGRADE_SUCCESS
 * @retval WM_UPGRADE_AGENT_IS_NOT_ACTIVE
 * */
int gm_agent_upgrade_validate_status(const char* connection_status);

/**
 * Check if WPK exists for this agent
 * @param platform platform of agent to validate
 * @param os_major OS major version of agent to validate
 * @param os_minor OS minor version of agent to validate
 * @param arch architecture of agent to validate
 * @param package_type variable used to store package type
 * @return return_code
 * @retval WM_UPGRADE_SUCCESS
 * @retval WM_UPGRADE_SYSTEM_NOT_SUPPORTED
 * @retval WM_UPGRADE_GLOBAL_DB_FAILURE
 * */
int gm_agent_upgrade_validate_system(const char *platform, const char *os_major, const char *os_minor, const char *arch, char **package_type);

/**
 * Check if agent is valid to upgrade
 * @param guardsarm_version guardsarm version of agent
 * @param platform platform of agent to validate
 * @param command wm_upgrade_command with the selected upgrade type
 * @param task pointer to task with the params
 * @return return_code
 * @retval WM_UPGRADE_SUCCESS
 * @retval WM_UPGRADE_NOT_MINIMAL_VERSION_SUPPORTED
 * @retval WM_UPGRADE_NEW_VERSION_LESS_OR_EQUAL_THAN_CURRENT
 * @retval WM_UPGRADE_NEW_VERSION_GREATER_MASTER
 * @retval WM_UPGRADE_GLOBAL_DB_FAILURE
 * */
int gm_agent_upgrade_validate_version(const char *guardsarm_version, const char *platform, gm_upgrade_command command, void *task)  __attribute__((nonnull(4)));

/**
 * Translate architecture based on platform and package type if necessary
 * @param platform Agent platform
 * @param package_type Package type
 * @param arch Agent architecture
 * @return Translated architecture
*/
char *gm_agent_upgrade_translate_arch(const char *platform, const char *package_type, char *arch);

/**
 * Check if a WPK exist for the upgrade version
 * @param agent_info structure with the agent information
 * @param task structure with the task information
 * @param wpk_repository_config char pointer with the repository url set in module config
 * @return return_code
 * @retval WM_UPGRADE_SUCCESS
 * @retval WM_UPGRADE_URL_NOT_FOUND
 * @retval WM_UPGRADE_WPK_VERSION_DOES_NOT_EXIST
 * @retval WM_UPGRADE_SYSTEM_NOT_SUPPORTED
 * */
int gm_agent_upgrade_validate_wpk_version(gm_agent_info *agent_info, gm_upgrade_task *task, const char *wpk_repository_config) __attribute__((nonnull(1, 2)));

/**
 * Check if WPK file exist or download it
 * @param task pointer to task with the params
 * @return return_code
 * @retval WM_UPGRADE_SUCCESS
 * @retval WM_UPGRADE_WPK_FILE_DOES_NOT_EXIST
 * @retval WM_UPGRADE_WPK_SHA1_DOES_NOT_MATCH
 * */
int gm_agent_upgrade_validate_wpk(const gm_upgrade_task *task) __attribute__((nonnull));

/**
 * Check if WPK custom file exist
 * @param task pointer to task with the params
 * @return return_code
 * @retval WM_UPGRADE_SUCCESS
 * @retval WM_UPGRADE_WPK_FILE_DOES_NOT_EXIST
 * */
int gm_agent_upgrade_validate_wpk_custom(const gm_upgrade_custom_task *task) __attribute__((nonnull));

/**
 * Validate a status response from the task manager module
 * @param response JSON to be validated
 * @param status string to save the status of the task
 * @param agent_id (optional) pointer to variable where the agent_id will be extracted and stored
 * Example formats:
 * 1. {
 *      "error": 0,
 *      "data": "Success",
 *      "agent": 1,
 *      "status": "Done"
 *  }
 * 2. {
 *      "error": 7,
 *      "data": "No task in DB",
 *      "agent": 2,
 *      "status": "Done"
 *  }
 * */
bool gm_agent_upgrade_validate_task_status_message(const cJSON *input_json, char **status, int *agent_id);

/**
 * Validate an upgrade response from the task manager module
 * @param input_json JSON to be validated
 * @param agent_id pointer to a variable where the agent_id will be stored
 * @param taks_id pointer to a variable where the task_id will be stored
 * @param data pointer to a variable where the data string will be stored
 * @return
 * @retval true if format is correct
 * @retval false if format is incorrrect
 * Example format:
 * {
 *      "error": 0,
 *      "data": "Success",
 *      "agent": 1,
 *      "task_id": 201
 *  }
 * */
bool gm_agent_upgrade_validate_task_ids_message(const cJSON *input_json, int *agent_id, int *task_id, char** data);

#endif
