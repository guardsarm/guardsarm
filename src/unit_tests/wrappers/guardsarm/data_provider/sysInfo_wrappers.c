/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#include "sysInfo_wrappers.h"
#include <stddef.h>
#include <stdarg.h>
#include <setjmp.h>
#include <stdint.h>
#include <cmocka.h>

int __wrap_sysinfo_hardware(cJSON ** js_result) {
    *js_result = mock_ptr_type(cJSON *);
    int ret = mock_type(int);
    return ret;
}

int __wrap_sysinfo_packages(cJSON ** js_result) {

    *js_result = mock_ptr_type(cJSON *);
    return mock_type(int);
}

int __wrap_sysinfo_os(cJSON ** js_result) {

    *js_result = mock_ptr_type(cJSON *);
    return mock_type(int);
}

int __wrap_sysinfo_processes(cJSON ** js_result) {

    *js_result = mock_ptr_type(cJSON *);
    return mock_type(int);
}

int __wrap_sysinfo_networks(cJSON ** js_result) {

    *js_result = mock_ptr_type(cJSON *);
    return mock_type(int);
}

int __wrap_sysinfo_ports(cJSON ** js_result) {

    *js_result = mock_ptr_type(cJSON *);
    return mock_type(int);
}

int __wrap_sysinfo_browser_extensions(cJSON ** js_result) {

    *js_result = mock_ptr_type(cJSON *);
    return mock_type(int);
}

void __wrap_sysinfo_free_result( __attribute__((unused)) cJSON ** js_data) {

    js_data = mock_ptr_type(cJSON **);
}
