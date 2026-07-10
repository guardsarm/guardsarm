/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#include "win_whodata_wrappers.h"
#include <stddef.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>

int __wrap_run_whodata_scan() {
    return mock();
}

int __wrap_set_winsacl(const char *dir, directory_t *configuration) {
    check_expected(dir);
    check_expected(configuration);

    return mock();
}

int __wrap_whodata_audit_start() {
    return 0;
}
