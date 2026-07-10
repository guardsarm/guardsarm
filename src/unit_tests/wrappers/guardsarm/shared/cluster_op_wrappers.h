/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */


#ifndef CLUSTER_OP_WRAPPERS_H
#define CLUSTER_OP_WRAPPERS_H

int __wrap_w_is_worker(void);

int __wrap_w_is_single_node(int* is_worker);

#endif
