/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#include "cluster_op_wrappers.h"
#include <stddef.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>

int __wrap_w_is_worker(void) {
    return mock();
}

int __wrap_w_is_single_node(int* is_worker) {
    if(is_worker) {
        *is_worker = mock();
    }

    return mock();
}
