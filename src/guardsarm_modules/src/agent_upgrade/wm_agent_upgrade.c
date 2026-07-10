/*
 * GuardSarm Module for Agent Upgrading
 * Copyright (C) 2026 GuardSarm, Inc.
 * July 3, 2020.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifdef GUARDSARM_UNIT_TESTING
// Remove static qualifier when unit testing
#define STATIC
#else
#define STATIC static
#endif

#include "wmodules.h"
#include "os_net.h"

#ifdef CLIENT
#include "wm_agent_upgrade_agent.h"
#else
#include "wm_agent_upgrade_manager.h"
#include "manager/wm_agent_upgrade_upgrades.h"
#endif

/**
 * Module main function. It won't return
 * */
#ifdef WIN32
STATIC DWORD WINAPI gm_agent_upgrade_main(void *arg);
#else
STATIC void* gm_agent_upgrade_main(gm_agent_upgrade* upgrade_config);
#endif
STATIC void gm_agent_upgrade_destroy(gm_agent_upgrade* upgrade_config);
STATIC cJSON *gm_agent_upgrade_dump(const gm_agent_upgrade* upgrade_config);
STATIC void gm_agent_upgrade_stop(gm_agent_upgrade* upgrade_config);

/* Context definition */
const gm_context GM_AGENT_UPGRADE_CONTEXT = {
    .name = AGENT_UPGRADE_WM_NAME,
    .start = (gm_routine)gm_agent_upgrade_main,
    .destroy = (void(*)(void *))gm_agent_upgrade_destroy,
    .dump = (cJSON * (*)(const void *))gm_agent_upgrade_dump,
    .sync = NULL,
    .stop = (void(*)(void *))gm_agent_upgrade_stop,
    .query = NULL,
};

#ifdef WIN32
STATIC DWORD WINAPI gm_agent_upgrade_main(void *arg) {
    gm_agent_upgrade* upgrade_config = (gm_agent_upgrade *)arg;
#else
STATIC void *gm_agent_upgrade_main(gm_agent_upgrade* upgrade_config) {
#endif
    #ifdef CLIENT
        gm_agent_upgrade_start_agent_module(&upgrade_config->agent_config, upgrade_config->enabled);
    #else
        gm_agent_upgrade_start_manager_module(&upgrade_config->manager_config, upgrade_config->enabled);
    #endif

#ifdef WIN32
    return 0;
#else
    return NULL;
#endif
}

STATIC void gm_agent_upgrade_stop(gm_agent_upgrade* upgrade_config) {
    (void)upgrade_config;
#ifndef CLIENT
    gm_agent_upgrade_stop_dispatch();
#endif
}

STATIC void gm_agent_upgrade_destroy(gm_agent_upgrade* upgrade_config) {
    mtinfo(GM_AGENT_UPGRADE_LOGTAG, GM_UPGRADE_MODULE_FINISHED);
    #ifndef CLIENT
    os_free(upgrade_config->manager_config.wpk_repository);
    #endif
    os_free(upgrade_config);
}

STATIC cJSON *gm_agent_upgrade_dump(const gm_agent_upgrade* upgrade_config){
    cJSON *root = cJSON_CreateObject();
    cJSON *gm_info = cJSON_CreateObject();

    if (upgrade_config->enabled) {
        cJSON_AddStringToObject(gm_info,"enabled","yes");
    } else {
        cJSON_AddStringToObject(gm_info,"enabled","no");
    }
    #ifndef CLIENT
    cJSON_AddNumberToObject(gm_info, "max_threads", upgrade_config->manager_config.max_threads);
    cJSON_AddNumberToObject(gm_info, "chunk_size", upgrade_config->manager_config.chunk_size);
    if (upgrade_config->manager_config.wpk_repository) {
        cJSON_AddStringToObject(gm_info, "wpk_repository", upgrade_config->manager_config.wpk_repository);
    }
    #else
    if (upgrade_config->agent_config.enable_ca_verification) {
        cJSON_AddStringToObject(gm_info,"ca_verification","yes");
    } else {
        cJSON_AddStringToObject(gm_info,"ca_verification","no");
    }
    if (wcom_ca_store) {
        cJSON *calist = cJSON_CreateArray();
        for (int i=0; wcom_ca_store[i]; i++) {
            cJSON_AddItemToArray(calist,cJSON_CreateString(wcom_ca_store[i]));
        }
        cJSON_AddItemToObject(gm_info,"ca_store",calist);
    }
    #endif
    cJSON_AddItemToObject(root,"agent-upgrade",gm_info);
    return root;
}
