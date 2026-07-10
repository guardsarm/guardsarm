/*
 * Copyright (C) 2026 GuardSarm, Inc.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include "io_wrappers.h"

char * wrap_mktemp_s(__attribute__((unused)) const char *path, __attribute__((unused)) ssize_t length) {
    return mock_type(char*);
}
