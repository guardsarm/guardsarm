/*
 * GuardSarm Module for content updates.
 * Copyright (C) 2026 GuardSarm, Inc.
 * May 1, 2023
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#include "wm_content_manager.h"
#include "content_manager.h"
#include <cJSON.h>
#include "logging_helper.h"
#include "sym_load.h"

static void gm_content_manager_destroy();
cJSON* gm_content_manager_dump();
static void gm_content_manager_stop();
static void* gm_content_manager_main();

void* content_manager_module = NULL;
content_manager_start_func content_manager_start_ptr = NULL;
content_manager_stop_func content_manager_stop_ptr = NULL;

const gm_context GM_CONTENT_MANAGER_CONTEXT = {
    .name = "content_manager",
    .start = (gm_routine)gm_content_manager_main,
    .destroy = (void (*)(void*))gm_content_manager_destroy,
    .dump = (cJSON * (*)(const void*)) gm_content_manager_dump,
    .sync = NULL,
    .stop = (void (*)(void*))gm_content_manager_stop,
    .query = NULL,
};

void* gm_content_manager_main()
{
    mtinfo(GM_CONTENT_MANAGER_LOGTAG, STARTUP_MSG, (int)getpid());

    if (content_manager_module = so_get_module_handle("content_manager"), content_manager_module)
    {
        content_manager_start_ptr = so_get_function_sym(content_manager_module, "content_manager_start");
        content_manager_stop_ptr = so_get_function_sym(content_manager_module, "content_manager_stop");

        if (content_manager_start_ptr)
        {
            content_manager_start_ptr(mtLoggingFunctionsWrapper);
        }
        else
        {
            mtwarn(GM_CONTENT_MANAGER_LOGTAG, "Unable to start content manager.");
        }
    }
    else
    {
        mtwarn(GM_CONTENT_MANAGER_LOGTAG, "Unable to load content_manager module.");
    }

    return NULL;
}

void gm_content_manager_destroy() {}

void gm_content_manager_stop()
{
    mtinfo(GM_CONTENT_MANAGER_LOGTAG, "Module finished.");

    if (content_manager_stop_ptr)
    {
        content_manager_stop_ptr();
    }
    else
    {
        mtwarn(GM_CONTENT_MANAGER_LOGTAG, "Unable to stop content manager.");
    }
}

gmodule* gm_content_manager_read()
{
    gmodule* module;

    os_calloc(1, sizeof(gmodule), module);
    module->context = &GM_CONTENT_MANAGER_CONTEXT;
    module->tag = strdup(module->context->name);

    mtdebug1(GM_CONTENT_MANAGER_LOGTAG, "Loaded content_manager module.");
    return module;
}

cJSON* gm_content_manager_dump()
{
    cJSON* root = cJSON_CreateObject();
    cJSON* gm_wd = cJSON_CreateObject();
    cJSON_AddStringToObject(gm_wd, "enabled", "yes");
    cJSON_AddItemToObject(root, "guardsarm_control", gm_wd);
    return root;
}

