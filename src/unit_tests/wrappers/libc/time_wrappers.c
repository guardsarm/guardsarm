/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#include "time_wrappers.h"
#include <stddef.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>
#include <string.h>

size_t __wrap_strftime(char * s, size_t max, __attribute__((unused)) const char * format,
                       __attribute__((unused)) const struct tm * tm) {
    strncpy(s, mock_type(char *), max);
    return mock();
}
