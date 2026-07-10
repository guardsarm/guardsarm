/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#include "errhandlingapi_wrappers.h"
#include <stddef.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>

DWORD wrap_GetLastError(VOID) {
    return mock();
}

void expect_GetLastError_call(int error_code) {
    will_return(wrap_GetLastError, error_code);
}
