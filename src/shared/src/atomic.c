/*
 * Copyright (C) 2026 GuardSarm, Inc.
 * December 18, 2018.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#include "shared.h"

int atomic_int_get(atomic_int_t *atomic) {
    assert(atomic != NULL);

    int retval = 0;

    w_mutex_lock(&atomic->mutex);
    retval = atomic->data;
    w_mutex_unlock(&atomic->mutex);

    return retval;
}

void atomic_int_set(atomic_int_t *atomic, int value) {
    assert(atomic != NULL);

    w_mutex_lock(&atomic->mutex);
    atomic->data = value;
    w_mutex_unlock(&atomic->mutex);
}

int atomic_int_inc(atomic_int_t *atomic) {
    assert(atomic != NULL);

    int retval = 0;
    w_mutex_lock(&atomic->mutex);
    atomic->data++;
    retval = atomic->data;
    w_mutex_unlock(&atomic->mutex);
    return retval;
};

int atomic_int_dec(atomic_int_t *atomic) {
    assert(atomic != NULL);

    int retval = 0;
    w_mutex_lock(&atomic->mutex);
    atomic->data--;
    retval = atomic->data;
    w_mutex_unlock(&atomic->mutex);
    return retval;
}
