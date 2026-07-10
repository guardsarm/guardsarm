/*
 * Copyright (C) 2026 GuardSarm, Inc.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#include <stddef.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>

#include "shared.h"
#include "bf_op.h"
#include "../../wrappers/common.h"

// Tests

void test_blowfish(void **state)
{
    const char *key = "test_key";
    const char *string = "test string";
    const int buffersize = 1024;
    char buffer1[buffersize];
    char buffer2[buffersize];

    assert_int_equal(OS_BF_Str(string, buffer1, key, buffersize, OS_ENCRYPT), 1);
    assert_int_equal(OS_BF_Str(buffer1, buffer2, key, buffersize, OS_DECRYPT), 1);

    assert_string_equal(buffer2, string);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_blowfish),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
