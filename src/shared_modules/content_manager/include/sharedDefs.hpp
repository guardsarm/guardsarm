/*
 * GuardSarm content manager
 * Copyright (C) 2026 GuardSarm, Inc.
 * March 25, 2023.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef _SHARED_DEFS_H
#define _SHARED_DEFS_H

#include "conditionSync.hpp"
#include "json.hpp"
#include <atomic>
#include <functional>
#include <string>

#define GM_CONTENTUPDATER "guardsarm-manager-modulesd:content-updater"

#include "loggerHelper.h"

using FileProcessingResult = std::tuple<int, std::string, bool>;
using FileProcessingCallback = std::function<FileProcessingResult(nlohmann::json message)>;

#endif // _SHARED_DEFS_H
