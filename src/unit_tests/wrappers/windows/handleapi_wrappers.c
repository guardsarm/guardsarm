/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#include "handleapi_wrappers.h"
#include <stddef.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>

WINBOOL wrap_CloseHandle(HANDLE hObject) {
    check_expected(hObject);
    return mock();
}

void expect_CloseHandle_call(HANDLE object, int ret) {
    expect_value(wrap_CloseHandle, hObject, object);
    will_return(wrap_CloseHandle, ret);
}

BOOL wrap_CreatePipe(PHANDLE hReadPipe,
                     PHANDLE hWritePipe,
                     LPSECURITY_ATTRIBUTES lpPipeAttributes,
                     DWORD nSize) {
    if (mock_type(BOOL)) {
        // Simulate valid handles when successful
        *hReadPipe = mock_type(HANDLE);
        *hWritePipe = mock_type(HANDLE);
        return TRUE;
    }
    return FALSE;
}

BOOL wrap_SetHandleInformation(HANDLE hObject,
                                DWORD dwMask,
                                DWORD dwFlags) {
    check_expected(hObject);
    check_expected(dwMask);
    check_expected(dwFlags);
    return mock_type(BOOL);
}
