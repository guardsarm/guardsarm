/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#include "exec_op_wrappers.h"
#include <stddef.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>


extern void write_date_storage();

int __wrap_wpclose(__attribute__((unused)) wfd_t * wfd) {
    return mock();
}

wfd_t *__wrap_wpopenl(__attribute__((unused)) const char * path, __attribute__((unused)) int flags, ...) {
    return mock_type(wfd_t *);
}

wfd_t *__wrap_wpopenv(__attribute__((unused)) const char * path,
                      __attribute__((unused)) char * const * argv,
                      __attribute__((unused)) int flags) {
    return mock_type(wfd_t *);
}
