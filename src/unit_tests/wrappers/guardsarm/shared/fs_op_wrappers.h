/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */


#ifndef FS_OP_WRAPPERS_H
#define FS_OP_WRAPPERS_H

#include <stdbool.h>
#include <fs_op.h>

bool __wrap_HasFilesystem(const char * path, fs_set set);

#endif
