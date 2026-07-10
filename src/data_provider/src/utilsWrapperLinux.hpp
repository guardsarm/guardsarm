/*
 * GuardSarm SysInfo
 * Copyright (C) 2026 GuardSarm, Inc.
 * December 22, 2021.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef _UTILS_WRAPPER_LINUX_H
#define _UTILS_WRAPPER_LINUX_H

#include <string>


class UtilsWrapperLinux final
{
    public:
        static std::string exec(const std::string& cmd, const size_t bufferSize = 128);
        static bool existsRegular(const std::string& path);
};

#endif // _UTILS_WRAPPER_LINUX_H
