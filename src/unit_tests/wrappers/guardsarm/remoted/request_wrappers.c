/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */


#include <stddef.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>
#include "request_wrappers.h"

int __wrap_req_save(const char * counter, const char * buffer, size_t length) {
    check_expected(counter);
    check_expected(buffer);
    check_expected(length);
    return mock();
}
