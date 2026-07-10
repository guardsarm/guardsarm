/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#include "wait_wrappers.h"
#include <stddef.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>
#include <string.h>
#include <stdio.h>

pid_t __wrap_waitpid(pid_t __pid, int * wstatus, int __options) {

    check_expected(__pid);
    check_expected(__options);
    *wstatus = mock_type(int);
    return mock_type(pid_t);
}
