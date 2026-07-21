/* Copyright (C) 2026 GuardSarm, Inc.
 * All right reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
*/

#include "os_regex.h"
#include "os_regex_internal.h"


int OS_StrStartsWith(const char *str, const char *pattern)
{
    while (*pattern) {
        if (*pattern++ != *str++) {
            return FALSE;
        }
    }

    return TRUE;
}
