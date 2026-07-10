/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef WIN32

#ifndef NOTIFY_OP_WRAPPERS_H
#define NOTIFY_OP_WRAPPERS_H

#include "notify_op.h"
#include "shared.h"

int __wrap_wnotify_modify(wnotify_t* notify, int fd, const woperation_t op);

int __wrap_wnotify_add(wnotify_t* notify, int fd, const woperation_t op);

#endif

#endif
