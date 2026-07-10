/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */


#ifndef SYSINFO_WRAPPERS_H
#define SYSINFO_WRAPPERS_H

#include "cJSON.h"

int __wrap_sysinfo_hardware(cJSON** js_result);
int __wrap_sysinfo_packages(cJSON** js_result);
int __wrap_sysinfo_os(cJSON** js_result);
int __wrap_sysinfo_processes(cJSON** js_result);
int __wrap_sysinfo_networks(cJSON** js_result);
int __wrap_sysinfo_ports(cJSON** js_result);
int __wrap_sysinfo_browser_extensions(cJSON ** js_result);
void __wrap_sysinfo_free_result(cJSON** js_data);

#endif
