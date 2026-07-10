/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#include "inotify_wrappers.h"
#include <stddef.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>


int __wrap_inotify_add_watch(__attribute__((unused)) int fd,
                             __attribute__((unused)) const char *pathname,
                             __attribute__((unused)) uint32_t mask) {
    return mock();
}

int __wrap_inotify_init() {
    return mock();
}

int __wrap_inotify_rm_watch() {
    return mock();
}
