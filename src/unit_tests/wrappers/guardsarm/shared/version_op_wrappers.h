/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */


#ifndef VERSION_OP_WRAPPERS_H
#define VERSION_OP_WRAPPERS_H

#include <stdbool.h>

int __wrap_compare_guardsarm_versions(const char *version1, const char *version2, bool compare_patch);

#endif
