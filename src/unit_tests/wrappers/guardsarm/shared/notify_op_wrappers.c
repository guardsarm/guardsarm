/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef WIN32

#include "notify_op_wrappers.h"
#include <stddef.h>
#include <stdarg.h>
#include <setjmp.h>
#include <stdint.h>
#include <cmocka.h>

int __wrap_wnotify_modify(wnotify_t * notify, int fd, const woperation_t op) {
    check_expected_ptr(notify);
    check_expected(fd);
    check_expected(op);
    return mock();
}

int __wrap_wnotify_add(wnotify_t * notify, int fd, const woperation_t op) {
    check_expected_ptr(notify);
    check_expected(fd);
    check_expected(op);
    return mock();
}

#endif
