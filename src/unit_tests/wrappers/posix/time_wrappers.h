/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */


#ifndef TIME_WRAPPERS_H
#define TIME_WRAPPERS_H

#include <time.h>
#include <sys/time.h>

time_t __wrap_time(time_t *t);
char *__wrap_ctime_r(const time_t *timep, char *buf);

void __wrap_gettimeofday(__attribute__((unused))struct timeval *__restrict __tv, __attribute__((unused)) void *__restrict __tz);

#endif // TIME_WRAPPERS_H
