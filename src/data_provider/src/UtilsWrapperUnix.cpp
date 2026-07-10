/*
 * GuardSarm SysInfo
 * Copyright (C) 2026 GuardSarm, Inc.
 * December 17, 2021.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#include <cerrno>
#include <cstring>
#include <system_error>

#include "UtilsWrapperUnix.hpp"

int UtilsWrapperUnix::createSocket(int domain, int type, int protocol)
{
    auto fd { socket(domain, type, protocol) };

    if (-1 == fd)
    {
        throw std::system_error{errno, std::system_category(), std::string(std::strerror(errno))};
    }

    return fd;
}

int UtilsWrapperUnix::ioctl(int fd, unsigned long request, char* argp)
{
    const auto retVal { ::ioctl(fd, request, argp) };

    if (-1 == retVal)
    {
        throw std::system_error{errno, std::system_category(), std::string(std::strerror(errno))};
    }

    return retVal;
}
