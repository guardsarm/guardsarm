/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#include "pthread_wrappers.h"
#include <stddef.h>
#include <stdarg.h>
#include <setjmp.h>
#include <stdint.h>
#include <cmocka.h>

void (*pthread_callback_ptr)(void) = NULL;

int __wrap_pthread_mutex_lock(__attribute__((unused)) pthread_mutex_t *x) {
    function_called();
    return 0;
}

int __wrap_pthread_mutex_unlock(__attribute__((unused)) pthread_mutex_t *x) {
    function_called();
    return 0;
}

int __wrap_pthread_rwlock_rdlock(__attribute__((unused)) pthread_rwlock_t *rwlock) {
    function_called();
    return 0;
}

int __wrap_pthread_rwlock_wrlock(__attribute__((unused)) pthread_rwlock_t *rwlock) {
    function_called();
    return 0;
}

int __wrap_pthread_rwlock_unlock(__attribute__((unused)) pthread_rwlock_t *rwlock) {
    function_called();
    return 0;
}

void __wrap_pthread_exit(__attribute__((unused)) void *value_ptr) {
    mock_assert(0, "pthread_exit", __FILE__, __LINE__);
}

int __wrap_pthread_cond_wait(pthread_cond_t *cond,pthread_mutex_t *mutex) {
    check_expected_ptr(cond);
    check_expected_ptr(mutex);
    // callback function to avoid infinite loops when testing
    if (pthread_callback_ptr)
        pthread_callback_ptr();
    return 0;
}

int __wrap_pthread_cond_signal(pthread_cond_t *cond) {
    check_expected_ptr(cond);
    return 0;
}
