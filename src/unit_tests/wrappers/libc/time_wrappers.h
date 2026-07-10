/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */


#ifndef TIME_WRAPPERS_H
#define TIME_WRAPPERS_H

#include <time.h>

size_t __wrap_strftime(char *s, size_t max, const char *format, const struct tm *tm);
#endif
