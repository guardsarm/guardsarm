/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef SHA256_OP_WRAPPERS_H
#define SHA256_OP_WRAPPERS_H

#include "shared.h"
#include <string.h>
#include <sys/types.h>

typedef char os_sha256[65];

int __wrap_OS_SHA256_String(const char* str, os_sha256 output);

#endif
