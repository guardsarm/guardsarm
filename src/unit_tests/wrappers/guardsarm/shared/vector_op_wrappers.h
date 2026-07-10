/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */


#ifndef VECTOR_OP_WRAPPERS_H
#define VECTOR_OP_WRAPPERS_H

#include "vector_op.h"

int __wrap_W_Vector_insert_unique(W_Vector *v, const char *element);

int __wrap_W_Vector_length(W_Vector *v);

#endif
