/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#include <stddef.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>

#include "rwlock_op_wrappers.h"

void __wrap_rwlock_lock_read(rwlock_t * rwlock) {
    (void)rwlock;
    function_called();
}

void __wrap_rwlock_lock_write(rwlock_t * rwlock) {
    (void)rwlock;
    function_called();
}

void __wrap_rwlock_unlock(rwlock_t * rwlock) {
    (void)rwlock;
    function_called();
}
