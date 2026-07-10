/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef WDB_METADATA_WRAPPERS_H
#define WDB_METADATA_WRAPPERS_H

#include "wdb.h"

int __wrap_wdb_user_version_get(wdb_t *wdb, int *version);

#endif
