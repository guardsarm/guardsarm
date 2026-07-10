/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#include "time_wrappers.h"
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <cmocka.h>
#include <string.h>

time_t __wrap_time(__attribute__ ((unused)) time_t *t) {
    return mock_type(time_t);
}

char *__wrap_ctime_r(const time_t *timep, char *buf) {
    check_expected(timep);

    strncpy(buf, mock_type(const char *), 26);

    return buf;
}

void __wrap_gettimeofday(struct timeval *__restrict __tv, __attribute__((unused)) void *__restrict __tz) {
        struct timeval *mocked_time = mock_ptr_type(struct timeval *);
        if (mocked_time && __tv) {
            *__tv = *mocked_time;
        }
}
