/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#include <stddef.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdio.h>
#include <string.h>
#include "kernel32_wrappers.h"

DWORD wrap_WaitForSingleObject(HANDLE hMutex, long value) {
    check_expected(hMutex);
    check_expected(value);
    return mock();
}

bool wrap_ReleaseMutex(HANDLE hMutex) {
    check_expected(hMutex);
    return mock();
}
