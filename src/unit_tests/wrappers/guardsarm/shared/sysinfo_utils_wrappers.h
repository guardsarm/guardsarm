/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef SYSINFO_UTILS_WRAPPERS_H
#define SYSINFO_UTILS_WRAPPERS_H

#include "sysinfo_utils.h"

bool __wrap_w_sysinfo_init(w_sysinfo_helpers_t * sysinfo);

bool __wrap_w_sysinfo_deinit(w_sysinfo_helpers_t * sysinfo);

cJSON * __wrap_w_sysinfo_get_processes(w_sysinfo_helpers_t * sysinfo);

cJSON * __wrap_w_sysinfo_get_os(w_sysinfo_helpers_t * sysinfo);

pid_t * __wrap_w_get_process_childs(w_sysinfo_helpers_t * sysinfo, pid_t parent_pid, unsigned int max_count);

char * __wrap_w_get_os_codename(w_sysinfo_helpers_t * sysinfo);

#endif
