/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#include "b64_wrappers.h"
#include <stddef.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>
#include <string.h>
#include <errno.h>

char *__wrap_encode_base64(int size, const char *src) {
    check_expected(size);
    check_expected(src);
    return mock_type(char *);
}

void expect_encode_base64(int size, const char *src, char * ret) {
    expect_value(__wrap_encode_base64, size, size);
    expect_string(__wrap_encode_base64, src, src);
    will_return(__wrap_encode_base64, strdup(ret));
}

char *__wrap_decode_base64(const char *src) {
    check_expected(src);
    return mock_type(char *);
}

void expect_decode_base64(const char *src, char * ret) {
    expect_string(__wrap_decode_base64, src, src);
    will_return(__wrap_decode_base64, strdup(ret));
}
