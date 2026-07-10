/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */


#ifndef HANDLEAPI_WRAPPERS_H
#define HANDLEAPI_WRAPPERS_H

#include <windows.h>

#undef CloseHandle
#define CloseHandle wrap_CloseHandle
#undef CreatePipe
#define CreatePipe wrap_CreatePipe
#undef SetHandleInformation
#define SetHandleInformation wrap_SetHandleInformation

WINBOOL wrap_CloseHandle (HANDLE hObject);

void expect_CloseHandle_call(HANDLE object, int ret);

BOOL wrap_CreatePipe(PHANDLE hReadPipe,
                     PHANDLE hWritePipe,
                     LPSECURITY_ATTRIBUTES lpPipeAttributes,
                     DWORD nSize);

BOOL wrap_SetHandleInformation(HANDLE hObject,
                                DWORD dwMask,
                                DWORD dwFlags);

#endif
