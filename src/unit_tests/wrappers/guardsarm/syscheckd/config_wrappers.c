/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#include "config_wrappers.h"
#include <stddef.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>

int OSHash_Add_ex_check_data = 1;

void __wrap_free_whodata_event(whodata_evt *w_evt) {
    if (OSHash_Add_ex_check_data) {
        check_expected(w_evt);
    }
}

cJSON * __wrap_getRootcheckConfig() {
    return mock_type(cJSON*);
}

cJSON * __wrap_getSyscheckConfig() {
    return mock_type(cJSON*);
}

cJSON * __wrap_getSyscheckInternalOptions() {
    return mock_type(cJSON*);
}
