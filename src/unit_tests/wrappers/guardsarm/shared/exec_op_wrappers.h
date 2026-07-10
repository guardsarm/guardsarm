/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef EXEC_OP_WRAPPERS_H
#define EXEC_OP_WRAPPERS_H

#ifdef WIN32
#include <processthreadsapi.h>
#endif
#include "exec_op.h"
#include <stdio.h>
#include <sys/types.h>

int __wrap_wpclose(wfd_t* wfd);

wfd_t* __wrap_wpopenl(const char* path, int flags, ...);

wfd_t* __wrap_wpopenv(const char* path, char* const* argv, int flags);

#endif
