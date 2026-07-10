/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */


#ifndef KERNEL32_WRAPPERS_WINDOWS_H
#define KERNEL32_WRAPPERS_WINDOWS_H

#include <stdbool.h>
#include <windows.h>

DWORD wrap_WaitForSingleObject(HANDLE hMutex, long value);

bool wrap_ReleaseMutex(HANDLE hMutex);

#undef WaitForSingleObject
#define WaitForSingleObject wrap_WaitForSingleObject
#undef ReleaseMutex
#define ReleaseMutex wrap_ReleaseMutex

#endif
