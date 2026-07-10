/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#include "synchapi_wrappers.h"
#include <stddef.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>

__attribute__((weak)) void wrap_Sleep_hook(__attribute__((unused)) DWORD dwMilliseconds) {}

VOID wrap_Sleep(DWORD dwMilliseconds) {
    check_expected(dwMilliseconds);
    wrap_Sleep_hook(dwMilliseconds);
}

HANDLE wrap_CreateEvent(LPSECURITY_ATTRIBUTES lpEventAttributes,
                        WINBOOL bManualReset,
                        WINBOOL bInitialState,
                        LPCSTR lpName) {
    check_expected(lpEventAttributes);
    check_expected(bManualReset);
    check_expected(bInitialState);
    check_expected(lpName);
    return mock_type(HANDLE);
}

DWORD wrap_WaitForSingleObjectEx(HANDLE hHandle, DWORD dwMilliseconds, BOOL bAlertable) {
    check_expected(hHandle);
    check_expected(dwMilliseconds);
    check_expected(bAlertable);
    return mock_type(DWORD);
}
