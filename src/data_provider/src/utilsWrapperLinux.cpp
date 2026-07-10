/*
 * GuardSarm SysInfo
 * Copyright (C) 2026 GuardSarm, Inc.
 * December 22, 2021.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#include "utilsWrapperLinux.hpp"
#include "cmdHelper.h"
#include <filesystem_wrapper.hpp>

std::string UtilsWrapperLinux::exec(const std::string& cmd, const size_t bufferSize)
{
    return Utils::exec(cmd, bufferSize);
}

bool UtilsWrapperLinux::existsRegular(const std::string& path)
{
    const file_system::FileSystemWrapper fs;

    return fs.is_regular_file(path);
}
