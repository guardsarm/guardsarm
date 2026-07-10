/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */


#ifndef WAIT_WRAPPERS_H
#define WAIT_WRAPPERS_H

#include <sys/types.h>
#include <sys/wait.h>

pid_t __wrap_waitpid(pid_t __pid, int * wstatus, int __options);

#endif // WAIT_WRAPPERS_H
