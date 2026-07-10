/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef WM_CONTROL_WRAPPERS_H
#define WM_CONTROL_WRAPPERS_H

#include <stdbool.h>
#include <stddef.h>
#include <sys/types.h>

size_t __wrap_wm_control_execute_action(const char *action, const char *service, char **output);
bool __wrap_wm_control_check_systemd(void);
pid_t __wrap_fork(void);

/* Linker-provided real symbol when --wrap,wm_control_check_systemd is enabled. */
extern bool __real_wm_control_check_systemd(void);

#endif
