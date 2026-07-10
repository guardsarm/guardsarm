/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#include "bqueue_op_wrappers.h"
#include <stddef.h>
#include <stdarg.h>
#include <setjmp.h>
#include <stdint.h>
#include <cmocka.h>

int __wrap_bqueue_push(bqueue_t * queue, const void * data, size_t length, unsigned flags) {
    check_expected_ptr(queue);
    check_expected(data);
    check_expected(length);
    check_expected(flags);
    return mock();
}

size_t __wrap_bqueue_peek(bqueue_t * queue, char * buffer, size_t length, unsigned flags) {
    check_expected_ptr(queue);
    check_expected(flags);
    if (mock()) {
        memcpy(buffer, mock_type(char *), length);
    }
    return mock();
}

int __wrap_bqueue_drop(bqueue_t * queue, size_t length) {
    check_expected_ptr(queue);
    check_expected(length);
    return mock();
}

void __wrap_bqueue_clear(bqueue_t * queue) {
    check_expected_ptr(queue);
}

size_t __wrap_bqueue_used(bqueue_t * queue) {
    check_expected_ptr(queue);
    return mock();
}
