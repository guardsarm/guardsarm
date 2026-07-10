/*
 * GuardSarm Module for Agent Upgrading
 * Copyright (C) 2026 GuardSarm, Inc.
 * July 30, 2020.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef GM_AGENT_UPGRADE_AGENT_H
#define GM_AGENT_UPGRADE_AGENT_H

#include "wm_agent_upgrade.h"

#ifdef WIN32
    #define GM_AGENT_UPGRADE_RESULT_FILE UPGRADE_DIR "\\upgrade_result"
#else
    #define GM_AGENT_UPGRADE_RESULT_FILE UPGRADE_DIR "/upgrade_result"
#endif

#define GM_AGENT_UPGRADE_RESULT_WAIT_TIME 30

typedef enum _wm_upgrade_agent_state {
    GM_UPGRADE_SUCCESSFUL = 0,
    GM_UPGRADE_FAILED_INTERMEDIATE,
    GM_UPGRADE_FAILED,
    GM_UPGRADE_MAX_STATE
} gm_upgrade_agent_state;

extern char **wcom_ca_store;

extern bool allow_upgrades;

/**
 * Start main loop of the upgrade module for the agent
 * @param agent_config Agent configuration parameters
 * @param enabled Wheter the module will allow or not upgrades
 * */
void gm_agent_upgrade_start_agent_module(const gm_agent_configs* agent_config, const int enabled) __attribute__((nonnull));

/**
 * Receives a string and process it with the available commands
 * Request format:
 *{
 *   "command": "upgrade",
 *   "parameters" : {
 *       "file" : "file_path",
 *       "installer" : "installer_path"
 *    }
 *}
 * @param buffer string with the information
 * @param output response to command
 * @return size of the response
 * */
size_t gm_agent_upgrade_process_command(const char *buffer, char **output);

#endif
