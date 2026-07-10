/*
 * GuardSarm SysInfo
 * Copyright (C) 2026 GuardSarm, Inc.
 * May 16, 2023.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#include "utilsWrapperMac.hpp"
#include "cmdHelper.h"

std::string UtilsWrapperMac::exec(const std::string& cmd, const size_t bufferSize)
{
    return Utils::exec(cmd, bufferSize);
}
