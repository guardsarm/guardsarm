/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */


#ifndef SIGNAL_WRAPPERS_H
#define SIGNAL_WRAPPERS_H

#include <signal.h>
#include <sys/types.h>

int __wrap_kill(pid_t pid, int sig);

#endif // SIGNAL_WRAPPERS_H
