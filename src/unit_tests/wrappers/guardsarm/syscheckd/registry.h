/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */
#include <cJSON.h>

#ifndef WIN_REGISTRY_WRAPPERS_H
#define WIN_REGISTRY_WRAPPERS_H

void __wrap_fim_registry_scan();

cJSON* __wrap_fim_dbsync_registry_value_json_event();

#endif
