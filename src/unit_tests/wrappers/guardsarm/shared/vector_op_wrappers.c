/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#include "vector_op_wrappers.h"
#include <stddef.h>
#include <stdarg.h>
#include <setjmp.h>
#include <stdint.h>
#include <cmocka.h>

int __wrap_W_Vector_insert_unique(W_Vector *v, const char *element) {
    check_expected_ptr(v);
    check_expected(element);

    return mock();
}

int __wrap_W_Vector_length(__attribute__((unused)) W_Vector *v) {
    return mock();
}
