/*
 * GuardSarm Module for routing messages to the right module
 * Copyright (C) 2026 GuardSarm, Inc.
 * May 1, 2023
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef _WM_ROUTER
#define _WM_ROUTER

#define GM_ROUTER_LOGTAG ARGV0 ":router"

#include "wmodules.h"

extern const gm_context GM_ROUTER_CONTEXT;

typedef struct gm_router_t
{
} gm_router_t;

gmodule* gm_router_read();

#endif /* _WM_ROUTER */
