/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdio.h>
#include <stdlib.h>
#include <shared.h>
#include "os_net.h"
#include "netcounter_wrappers.h"

void __wrap_rem_setCounter(int fd, size_t counter) {
    check_expected(fd);
    check_expected(counter);
}

size_t __wrap_rem_getCounter(int fd) {
    check_expected(fd);
    return mock();
}
