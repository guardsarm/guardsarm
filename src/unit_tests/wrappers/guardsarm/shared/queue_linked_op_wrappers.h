/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef LINKED_QUEUE_OP_WRAPPERS_H
#define LINKED_QUEUE_OP_WRAPPERS_H

#include "queue_linked_op.h"
#include "shared.h"

w_linked_queue_node_t* __wrap_linked_queue_push_ex(w_linked_queue_t* queue, void* data);

void* __wrap_linked_queue_pop_ex(w_linked_queue_t* queue);
#endif
