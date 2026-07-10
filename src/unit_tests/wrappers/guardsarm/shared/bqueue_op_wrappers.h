/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef BQUEUE_OP_WRAPPERS_H
#define BQUEUE_OP_WRAPPERS_H

#include "bqueue_op.h"
#include "shared.h"

int __wrap_bqueue_push(bqueue_t* queue, const void* data, size_t length, unsigned flags);

size_t __wrap_bqueue_peek(bqueue_t* queue, char* buffer, size_t length, unsigned flags);

int __wrap_bqueue_drop(bqueue_t* queue, size_t length);

void __wrap_bqueue_clear(bqueue_t* queue);

size_t __wrap_bqueue_used(bqueue_t* queue);

#endif
