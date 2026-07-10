/*
 * GuardSarm Module for Agent Upgrading
 * Copyright (C) 2026 GuardSarm, Inc.
 * July 15, 2020.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef GM_AGENT_UPGRADE_H
#define GM_AGENT_UPGRADE_H

#include "wmodules_def.h"

#define GM_AGENT_UPGRADE_LOGTAG ARGV0 ":" AGENT_UPGRADE_WM_NAME

#define GM_UPGRADE_WPK_REPO_URL_3_X "packages.guardsarmsiem.com/wpk/"
#define GM_UPGRADE_WPK_REPO_URL "packages.guardsarmsiem.com/%d.x/wpk/"
#define GM_UPGRADE_CHUNK_SIZE 32768
#define GM_UPGRADE_CHUNK_SIZE_MIN 64
#define GM_UPGRADE_CHUNK_SIZE_MAX 60000
#define GM_UPGRADE_MAX_THREADS 8
#define GM_UPGRADE_WAIT_START 30
#define GM_UPGRADE_WAIT_MAX 3600
#define GM_UPGRADE_WAIT_FACTOR_INCREASE 2.0

/**
 * Configurations on agent side
 */
typedef struct _wm_agent_configs {
    unsigned int upgrade_wait_start;
    unsigned int upgrade_wait_max;
    float upgrade_wait_factor_increase;
    unsigned int enable_ca_verification;
} gm_agent_configs;

/**
 * Configuration only for manager
 */
typedef struct _wm_manager_configs {
    unsigned int max_threads;
    unsigned int chunk_size;
    char *wpk_repository;
} gm_manager_configs;

typedef struct _wm_agent_upgrade {
    int enabled:1;
    gm_agent_configs agent_config;
    gm_manager_configs manager_config;
} gm_agent_upgrade;

// Parse XML configuration
int gm_agent_upgrade_read(const OS_XML *xml, xml_node **nodes, gmodule *module);

extern const gm_context GM_AGENT_UPGRADE_CONTEXT;   // Context

#endif
