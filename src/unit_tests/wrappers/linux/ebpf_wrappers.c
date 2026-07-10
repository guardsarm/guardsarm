/* Copyright (C) 2025, Wazuh Inc.
 * All right reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#include "syscheck.h"
#include "ebpf_wrappers.h"
#include <setjmp.h>
#include <cmocka.h>


int __wrap_ebpf_whodata_healthcheck() {
    // Your function definition goes here
    return mock_type(int);
}

typedef void (*FunctionPtr)();

void __wrap_fimebpf_initialize(const char* config_dir,
                                FunctionPtr get_user,
                                FunctionPtr get_group,
                                FunctionPtr whodata_event,
                                FunctionPtr free_whodata_event,
                                FunctionPtr loggingFunction,
                                FunctionPtr abspath,
                                FunctionPtr is_shutdown,
                                syscheck_config syscheck) {
}
