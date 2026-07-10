/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef IO_WIN_WRAPPERS_H
#define IO_WIN_WRAPPERS_H

#include <io.h>

#undef _mktemp_s
#define _mktemp_s  wrap_mktemp_s

char * wrap_mktemp_s(const char *path, ssize_t length);

#endif
