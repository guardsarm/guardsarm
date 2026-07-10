/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#include "version_op_wrappers.h"
#include <stddef.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>

int __wrap_compare_guardsarm_versions(const char *version1, const char *version2, bool compare_patch) {
    check_expected(version1);
    check_expected(version2);
    check_expected(compare_patch);

    return mock();
}
