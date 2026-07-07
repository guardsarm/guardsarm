/*
 * GuardSarm content manager
 * Copyright (C) 2015, Wazuh Inc.
 * Copyright (C) 2026, GuardSarm.
 * March 25, 2023.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation.
 */

#ifndef _SHARED_DEFS_H
#define _SHARED_DEFS_H

#include "conditionSync.hpp"
#include "json.hpp"
#include <atomic>
#include <functional>
#include <string>

#define WM_CONTENTUPDATER "guardsarm-manager-modulesd:content-updater"

#include "loggerHelper.h"

using FileProcessingResult = std::tuple<int, std::string, bool>;
using FileProcessingCallback = std::function<FileProcessingResult(nlohmann::json message)>;

#endif // _SHARED_DEFS_H
