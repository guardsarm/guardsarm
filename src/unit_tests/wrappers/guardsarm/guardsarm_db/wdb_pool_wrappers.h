/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef WDB_POOL_WRAPPERS_H
#define WDB_POOL_WRAPPERS_H

#include "wdb.h"

wdb_t* __wrap_wdb_pool_get(const char* name);

wdb_t* __wrap_wdb_pool_get_or_create(const char* name);

void __wrap_wdb_pool_leave(__attribute__((unused)) wdb_t* node);

char** __wrap_wdb_pool_keys();

void __wrap_wdb_pool_clean();

#endif
