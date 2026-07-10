/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */


#ifndef ERRHANDLINGAPI_WRAPPERS_H
#define ERRHANDLINGAPI_WRAPPERS_H

#include <windows.h>

#undef GetLastError
#define GetLastError wrap_GetLastError

DWORD wrap_GetLastError(VOID);

void expect_GetLastError_call(int error_code);

#endif
