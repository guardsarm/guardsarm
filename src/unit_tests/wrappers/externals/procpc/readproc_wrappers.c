/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#include "readproc_wrappers.h"
#include <stddef.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>


void __wrap_closeproc(PROCTAB* PT) {
    check_expected(PT);
}

void __wrap_freeproc(proc_t* p) {
    check_expected(p);
}

PROCTAB* __wrap_openproc(int flags, ...) {
    check_expected(flags);
    return mock_type(PROCTAB*);
}

proc_t* __wrap_readproc(PROCTAB *restrict const PT,
                        proc_t *restrict p) {
    check_expected(PT);
    check_expected(p);

    return mock_type(proc_t*);
}
