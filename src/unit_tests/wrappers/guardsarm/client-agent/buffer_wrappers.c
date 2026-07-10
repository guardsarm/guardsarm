/*
 * Copyright (C) 2026 GuardSarm, Inc.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#include "buffer_wrappers.h"
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>

int __wrap_w_agentd_get_buffer_lenght() {
    return mock();
}
