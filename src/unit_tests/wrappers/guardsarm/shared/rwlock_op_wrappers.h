/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef RWLOCK_OP_WRAPPERS_H
#define RWLOCK_OP_WRAPPERS_H

#include <rwlock_op.h>

void __wrap_rwlock_lock_read(rwlock_t * rwlock);
void __wrap_rwlock_lock_write(rwlock_t * rwlock);
void __wrap_rwlock_unlock(rwlock_t * rwlock);

#endif
