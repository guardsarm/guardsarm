/*
 * GuardSarm shared modules utils
 * Copyright (C) 2026 GuardSarm, Inc.
 * September 27, 2022.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef _CJSON_SMART_DELETER_HPP
#define _CJSON_SMART_DELETER_HPP

#include "cJSON.h"
#include "customDeleter.hpp"

struct CJsonSmartFree final : CustomDeleter<decltype(&cJSON_free), cJSON_free>
{
};
struct CJsonSmartDeleter final : CustomDeleter<decltype(&cJSON_Delete), cJSON_Delete>
{
};

#endif // _CJSON_SMART_DELETER_HPP
