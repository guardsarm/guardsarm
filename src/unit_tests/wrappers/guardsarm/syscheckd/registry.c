/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#include "registry.h"
#include <stddef.h>
#include <stdarg.h>
#include <setjmp.h>
#include <stdint.h>
#include <cmocka.h>
#include <cJSON.h>

void __wrap_fim_registry_scan() {
    return;
}

cJSON* __wrap_fim_dbsync_registry_value_json_event(){
    return mock_ptr_type(cJSON*);
}
