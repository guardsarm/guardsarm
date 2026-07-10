/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */


#ifndef PTHREADS_OP_WRAPPERS_H
#define PTHREADS_OP_WRAPPERS_H

int __wrap_CreateThread(void * (*function_pointer)(void *), void *data);

#endif
