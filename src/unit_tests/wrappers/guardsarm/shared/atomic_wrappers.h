/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */
#ifndef ATOMIC_WRAPPERS
#define ATOMIC_WRAPPERS

#include "atomic.h"

int __wrap_atomic_int_get(atomic_int_t* atomic);

void __wrap_atomic_int_set(atomic_int_t* atomic, __attribute__((unused)) int value);

int __wrap_atomic_int_inc(atomic_int_t* atomic);

int __wrap_atomic_int_dec(atomic_int_t* atomic);

#endif // ATOMIC_WRAPPERS
