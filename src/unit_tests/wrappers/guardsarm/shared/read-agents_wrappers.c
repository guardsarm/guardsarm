/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#include "read-agents_wrappers.h"
#include <stddef.h>
#include <stdarg.h>
#include <setjmp.h>
#include <stdint.h>
#include <cmocka.h>

int __wrap_connect_to_remoted() {
    return mock();
}

int __wrap_send_msg_to_agent(int msocket, const char *msg, const char *agt_id, const char *exec) {
    check_expected(msocket);
    check_expected(msg);
    check_expected(agt_id);
    check_expected_ptr(exec);

    return mock();
}
