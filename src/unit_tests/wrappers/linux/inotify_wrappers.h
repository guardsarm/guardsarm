/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */


#ifndef INOTIFY_WRAPPERS_H
#define INOTIFY_WRAPPERS_H

#include <stdint.h>

int __wrap_inotify_add_watch(int fd,
                             const char *pathname,
                             uint32_t mask);

int __wrap_inotify_init();

int __wrap_inotify_rm_watch();

#endif
