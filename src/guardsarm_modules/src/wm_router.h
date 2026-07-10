/*
 * GuardSarm Module for routing messages to the right module
 * Copyright (C) 2026 GuardSarm, Inc.
 * May 1, 2023
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef _WM_ROUTER
#define _WM_ROUTER

#define WM_ROUTER_LOGTAG ARGV0 ":router"

#include "wmodules.h"

extern const wm_context WM_ROUTER_CONTEXT;

typedef struct wm_router_t
{
} wm_router_t;

wmodule* wm_router_read();

#endif /* _WM_ROUTER */
