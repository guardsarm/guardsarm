/*
 * GuardSarm Module for routing messages to the right module
 * Copyright (C) 2026 GuardSarm, Inc.
 * May 1, 2023
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef _WM_INVENTORY_SYNC_H
#define _WM_INVENTORY_SYNC_H

#define WM_INVENTORY_SYNC_LOGTAG ARGV0 ":inventory-sync"

#include "wmodules.h"

extern const wm_context WM_INVENTORY_SYNC_CONTEXT;

typedef struct wm_inventory_sync_t
{
    cJSON* inventory_sync;
} wm_inventory_sync_t;

wmodule* wm_inventory_sync_read();

#endif /* _WM_INVENTORY_SYNC_H */
