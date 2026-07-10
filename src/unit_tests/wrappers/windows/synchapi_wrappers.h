/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */


#ifndef SYNCHAPI_WRAPPERS_H
#define SYNCHAPI_WRAPPERS_H

#include <windows.h>

#define Sleep wrap_Sleep
#undef CreateEvent
#define CreateEvent wrap_CreateEvent
#undef WaitForSingleObjectEx
#define WaitForSingleObjectEx wrap_WaitForSingleObjectEx

VOID wrap_Sleep(DWORD dwMilliseconds);
void wrap_Sleep_hook(DWORD dwMilliseconds);

HANDLE wrap_CreateEvent(LPSECURITY_ATTRIBUTES lpEventAttributes,
                        WINBOOL bManualReset,
                        WINBOOL bInitialState,
                        LPCSTR lpName);

DWORD wrap_WaitForSingleObjectEx(HANDLE hHandle, DWORD dwMilliseconds, BOOL bAlertable);

#endif
