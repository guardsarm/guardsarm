/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#include "atomic_wrappers.h"
#include <stddef.h>
#include <stdarg.h>
#include <setjmp.h>
#include <stdint.h>
#include <cmocka.h>

int __wrap_atomic_int_get(atomic_int_t *atomic) {
    check_expected_ptr(atomic);
    return mock();
}

void __wrap_atomic_int_set(atomic_int_t *atomic, __attribute__((unused)) int value) {
    check_expected_ptr(atomic);
    atomic->data = mock();
}

int __wrap_atomic_int_inc(atomic_int_t *atomic) {
    check_expected_ptr(atomic);
    return mock();
}

int __wrap_atomic_int_dec(atomic_int_t *atomic) {
    check_expected_ptr(atomic);
    return mock();
}
