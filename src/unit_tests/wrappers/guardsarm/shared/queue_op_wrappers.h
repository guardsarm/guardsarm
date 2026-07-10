/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef QUEUE_OP_WRAPPERS_H
#define QUEUE_OP_WRAPPERS_H

#include "queue_op.h"

int __wrap_queue_push_ex(w_queue_t* queue, void* data);

int __wrap_queue_full(const w_queue_t* queue);

void* __wrap_queue_pop_ex(w_queue_t* queue);

void* __wrap_queue_pop_ex_timedwait(w_queue_t* queue, const struct timespec* abstime);

#endif
