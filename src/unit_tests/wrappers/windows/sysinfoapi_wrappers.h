/*
 * Copyright (C) 2026 GuardSarm, Inc.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef SYSINFOAPI_WRAPPERS_H
#define SYSINFOAPI_WRAPPERS_H

#include <windows.h>

#define GetSystemTime wrap_GetSystemTime

VOID wrap_GetSystemTime(LPSYSTEMTIME lpSystemTime);

#endif
