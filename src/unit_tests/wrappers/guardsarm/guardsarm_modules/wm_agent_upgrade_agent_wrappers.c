/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#include "../../common.h"
#include "wm_agent_upgrade_agent_wrappers.h"
#include <stddef.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>

void __wrap_wm_agent_upgrade_start_agent_module(const gm_agent_configs* agent_config, const int enabled) {
    check_expected(agent_config);
    check_expected(enabled);
}

size_t __wrap_wm_agent_upgrade_process_command(const char *buffer, char **output) {
    check_expected(buffer);
    *output = mock_type(char*);

    return mock();
}
