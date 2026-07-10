/*
 * Copyright (C) 2026 GuardSarm, Inc.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdio.h>
#include <string.h>

#include "os_err.h"
#include "shared.h"
#include "auth.h"
#include "sec.h"

#include "../wrappers/guardsarm/shared/debug_op_wrappers.h"
#include "../wrappers/guardsarm/os_auth/os_auth_wrappers.h"
#include "../wrappers/guardsarm/shared/randombytes_wrappers.h"

/* tests */

static void test_w_generate_random_pass_success(void **state) {
    char* result = NULL;

    will_return(__wrap_os_random, 146557);
    will_return(__wrap_os_random, 314159);
    will_return(__wrap_GetRandomNoise, strdup("GuardSarm"));
    will_return(__wrap_GetRandomNoise, strdup("The Open Source Security Platform"));
    will_return(__wrap_time, 1655254875);
    will_return_always(__wrap_getuname, "Linux |ubuntu-focal |5.4.0-92-generic |#103-Ubuntu SMP Fri Nov 26 16:13:00 UTC 2021 "
                                        "|x86_64 [Ubuntu|ubuntu: 20.04.2 LTS (Focal Fossa)] - GuardSarm v4.3.4");

    result = w_generate_random_pass();

    assert_string_equal(result, "6e0d9a4188ac9de8fa695bd96e276090");
    os_free(result);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_w_generate_random_pass_success),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
