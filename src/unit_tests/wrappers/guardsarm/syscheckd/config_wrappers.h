/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef SYSCHECKD_CONFIG_WRAPPERS_H
#define SYSCHECKD_CONFIG_WRAPPERS_H

#include "syscheck.h"
#include <cJSON.h>

void __wrap_free_whodata_event(whodata_evt* w_evt);

cJSON* __wrap_getRootcheckConfig();

cJSON* __wrap_getSyscheckConfig();

cJSON* __wrap_getSyscheckInternalOptions();

#endif
