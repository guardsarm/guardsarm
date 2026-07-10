/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */
#ifndef RUN_REALTIME_WRAPPERS_H
#define RUN_REALTIME_WRAPPERS_H

#include "syscheck-config.h"

int __wrap_realtime_adddir(const char *dir,
                           directory_t *configuration);

int __wrap_realtime_start();

/**
 * @brief This function loads the expect and will_return calls for the wrapper of realtime_adddir
 */
void expect_realtime_adddir_call(const char *path, int ret);

int __wrap_fim_add_inotify_watch(const char *dir,
                                 const directory_t *configuration);

void __wrap_realtime_process();

void __wrap_realtime_sanitize_watch_map();

#endif
