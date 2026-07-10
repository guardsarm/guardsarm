/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef SIGNATURE_WRAPPERS_H
#define SIGNATURE_WRAPPERS_H

#include "shared.h"
#include <string.h>

int __wrap_w_wpk_unsign(const char* source, const char* target, const char** ca_store);

#endif
