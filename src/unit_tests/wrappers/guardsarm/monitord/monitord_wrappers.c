/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#include "monitord_wrappers.h"
#include <stddef.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>

void __wrap_w_rotate_log(__attribute__((unused)) int compress,
                         __attribute__((unused)) int keep_log_days,
                         __attribute__((unused)) int new_day,
                         __attribute__((unused)) int rotate_json,
                         __attribute__((unused)) int daily_rotations) {
    return;
}
