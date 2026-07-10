/*
 * GuardSarm Module for update content for modules.
 * Copyright (C) 2026 GuardSarm, Inc.
 * May 1, 2023
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef _WM_CONTENT_MANAGER
#define _WM_CONTENT_MANAGER

#define GM_CONTENT_MANAGER_LOGTAG ARGV0 ":content_manager"

#include "wmodules.h"

extern const gm_context GM_CONTENT_MANAGER_CONTEXT;

typedef struct gm_content_manager_t
{
    unsigned int enabled : 1;
    unsigned int run_on_start : 1;
} gm_content_manager_t;

gmodule* gm_content_manager_read();

#endif /* _WM_CONTENT_MANAGER */
