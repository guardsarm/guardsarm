/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#include <time.h>
#include <stdio.h>
#include <stddef.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>
#include <string.h>
#include "defs.h"

time_t time_mock_value;
int test_mode = 0;
int activate_full_db = 0;

int FOREVER() {
    return 1;
}

int __wrap_FOREVER() {
    return mock();
}

time_t wrap_time (__attribute__ ((__unused__)) time_t *t) {
    return time_mock_value;
}
