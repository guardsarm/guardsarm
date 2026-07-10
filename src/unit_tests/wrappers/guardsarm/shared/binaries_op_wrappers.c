/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#include "binaries_op_wrappers.h"
#include <string.h>
#include <stddef.h>
#include <stdarg.h>
#include <setjmp.h>
#include <stdint.h>
#include <cmocka.h>

int __wrap_get_binary_path(const char *command, char **path) {
    check_expected(command);
    *path = (char*)mock_ptr_type(char*);

    return mock_type(int);
}
