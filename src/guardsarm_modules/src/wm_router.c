/*
 * GuardSarm Module for routing messages.
 * Copyright (C) 2026 GuardSarm, Inc.
 * May 1, 2023
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#include "wm_router.h"
#include <cJSON.h>
#include "router.h"
#include "sym_load.h"

static void gm_router_destroy();
cJSON* gm_router_dump();
static void gm_router_stop();
static void* gm_router_main();

void* router_module = NULL;

router_initialize_func router_initialize_ptr = NULL;
router_start_func router_start_ptr = NULL;
router_stop_func router_stop_ptr = NULL;

const gm_context GM_ROUTER_CONTEXT = {
    .name = "router",
    .start = (gm_routine)gm_router_main,
    .destroy = (void (*)(void*))gm_router_destroy,
    .dump = (cJSON * (*)(const void*)) gm_router_dump,
    .sync = NULL,
    .stop = (void (*)(void*))gm_router_stop,
    .query = NULL,
};

void* gm_router_main()
{
    mtinfo(GM_ROUTER_LOGTAG, STARTUP_MSG, (int)getpid());
    if (router_module = so_get_module_handle("router"), router_module)
    {
        router_start_ptr = so_get_function_sym(router_module, "router_start");
        router_stop_ptr = so_get_function_sym(router_module, "router_stop");
        router_initialize_ptr = so_get_function_sym(router_module, "router_initialize");

        if (router_initialize_ptr)
        {
            router_initialize_ptr(taggedLogFunction);
        }
        else
        {
            mtwarn(GM_ROUTER_LOGTAG, "Unable to initialize router module.");
            return NULL;
        }

        if (router_start_ptr)
        {
            router_start_ptr();
        }
        else
        {
            mtwarn(GM_ROUTER_LOGTAG, "Unable to start router module.");
            return NULL;
        }
    }
    else
    {
        mtwarn(GM_ROUTER_LOGTAG, "Unable to load router module.");
        return NULL;
    }

    return NULL;
}

void gm_router_destroy() {}

void gm_router_stop()
{
    mtinfo(GM_ROUTER_LOGTAG, "Module finished.");
    if (router_stop_ptr)
    {
        router_stop_ptr();
    }
    else
    {
        mtwarn(GM_ROUTER_LOGTAG, "Unable to stop router module.");
    }
}

gmodule* gm_router_read()
{
    gmodule* module;

    os_calloc(1, sizeof(gmodule), module);
    module->context = &GM_ROUTER_CONTEXT;
    module->tag = strdup(module->context->name);
    mtdebug1(GM_ROUTER_LOGTAG, "Loaded router module.");
    return module;
}

cJSON* gm_router_dump()
{
    cJSON* root = cJSON_CreateObject();
    cJSON* gm_wd = cJSON_CreateObject();
    cJSON_AddStringToObject(gm_wd, "enabled", "yes");
    cJSON_AddItemToObject(root, "guardsarm_control", gm_wd);
    return root;
}
