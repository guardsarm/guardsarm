/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */


#ifndef PTHREAD_WRAPPERS_H
#define PTHREAD_WRAPPERS_H

#include <pthread.h>

int __wrap_pthread_mutex_lock(pthread_mutex_t *x);

int __wrap_pthread_mutex_unlock(pthread_mutex_t *x);

int __wrap_pthread_rwlock_rdlock(pthread_rwlock_t *rwlock);

int __wrap_pthread_rwlock_wrlock(pthread_rwlock_t *rwlock);

int __wrap_pthread_rwlock_unlock(pthread_rwlock_t *rwlock);

void __wrap_pthread_exit(void *value_ptr);

int __wrap_pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex);

int __wrap_pthread_cond_signal(pthread_cond_t *cond);

extern void (*pthread_callback_ptr)(void);

#endif
