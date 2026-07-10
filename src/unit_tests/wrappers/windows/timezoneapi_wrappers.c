/*
 * Copyright (C) 2026 GuardSarm, Inc.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */
#include "timezoneapi_wrappers.h"
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

WINBOOL wrap_FileTimeToSystemTime(CONST FILETIME *lpFileTime,
                                  LPSYSTEMTIME lpSystemTime) {
    check_expected(lpFileTime);
    memcpy(lpSystemTime, mock_type(LPSYSTEMTIME), sizeof(SYSTEMTIME));
    return mock();
}
