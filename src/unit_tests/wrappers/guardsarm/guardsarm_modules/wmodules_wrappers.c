/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#include "wmodules_wrappers.h"
#include <stddef.h>
#include <stdarg.h>
#include <setjmp.h>
#include <stdint.h>
#include <cmocka.h>
#include <string.h>

int __wrap_wm_sendmsg(int usec, int queue, const char *message, const char *locmsg, char loc) {
    check_expected(usec);
    check_expected(queue);
    check_expected(message);
    check_expected(locmsg);
    check_expected(loc);

    return mock();
}

int __wrap_wm_state_io(const char * tag,
                       int op,
                       void *state,
                       size_t size) {
    check_expected(tag);
    check_expected(op);
    check_expected_ptr(state);
    check_expected(size);
    int ret = mock();
    if (!ret) {
        memcpy(state, mock_type(void *), sizeof(*state));
    }
    return ret;
}

int __wrap_wm_validate_command(const char *command, const char *digest, crypto_type ctype) {
    check_expected(command);
    check_expected(digest);
    check_expected(ctype);

    return mock_type(int);
}

void expect_wm_validate_command(const char *command, const char *digest, crypto_type ctype, int ret) {
    expect_string(__wrap_wm_validate_command, command, command);
    expect_string(__wrap_wm_validate_command, digest, digest);
    expect_value(__wrap_wm_validate_command, ctype, ctype);
    will_return(__wrap_wm_validate_command, ret);
}
