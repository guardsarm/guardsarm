/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef SYSTEM_CALLS_WRAPPERS_H
#define SYSTEM_CALLS_WRAPPERS_H

//#undef _mktemp_s
//#define _mktemp_s  wrap_mktemp_s

char * wrap_getenv(const char *name);

#endif
