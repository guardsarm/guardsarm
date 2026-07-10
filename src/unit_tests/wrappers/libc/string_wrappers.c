/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#include "string_wrappers.h"
#include <stddef.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>


char *__wrap_strerror(__attribute__((unused)) int __errnum) {
    return mock_type(char*);
}

size_t __wrap_strlen(const char *s) {
    check_expected(s);
    return mock();
}
