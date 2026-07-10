/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef JSON_OP_WRAPPERS_H
#define JSON_OP_WRAPPERS_H

#include "shared.h"

cJSON* __wrap_json_fread(const char* path, char retry);
int __wrap_json_fwrite(const char* path, const cJSON* item);

int* __wrap_json_parse_agents(const cJSON* agents);

int* __wrap_json_parse_agents(const cJSON* agents);

#endif
