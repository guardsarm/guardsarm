/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#include "syscom_wrappers.h"
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>

size_t __wrap_syscom_dispatch(char * command, size_t length, char ** output) {
    check_expected(command);

    *output = mock_type(char*);
    return mock();
}
