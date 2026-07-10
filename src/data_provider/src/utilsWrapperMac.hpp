/*
 * GuardSarm SysInfo
 * Copyright (C) 2026 GuardSarm, Inc.
 * May 16, 2023.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef _UTILS_WRAPPER_MAC_H
#define _UTILS_WRAPPER_MAC_H

#include <string>


class UtilsWrapperMac final
{
    public:
        static std::string exec(const std::string& cmd, const size_t bufferSize = 128);
};

#endif // _UTILS_WRAPPER_MAC_H
