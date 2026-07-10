
/*
 * Copyright (C) 2026 GuardSarm, Inc.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef TIMEZONEAPI_WRAPPERS_H
#define TIMEZONEAPI_WRAPPERS_H

#include <windows.h>

#define FileTimeToSystemTime wrap_FileTimeToSystemTime

WINBOOL wrap_FileTimeToSystemTime(CONST FILETIME *lpFileTime,
                                  LPSYSTEMTIME lpSystemTime);


#endif
