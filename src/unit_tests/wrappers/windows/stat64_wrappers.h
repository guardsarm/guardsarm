/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */
#ifndef STAT64_WRAPPERS_H
#define STAT64_WRAPPERS_H

#include <sys/stat.h>

#define _stat64(x, y) wrap__stat64(x, y)

int wrap__stat64(const char * __file, struct _stat64 * __buf);

#endif
