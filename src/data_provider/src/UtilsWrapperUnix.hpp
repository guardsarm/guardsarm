/*
 * GuardSarm SYSINFO
 * Copyright (C) 2026 GuardSarm, Inc.
 * December 17, 2021.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef _UTILS_WRAPPER_UNIX_H
#define _UTILS_WRAPPER_UNIX_H

#include <stdexcept>

#include <sys/socket.h>
#include <net/if.h>
#include <unistd.h>
#include <stropts.h>

class UtilsWrapperUnix
{
    public:
        static int createSocket(int domain, int type, int protocol);
        static int ioctl(int fd, unsigned long request, char* argp);
};

#endif // _UTILS_WRAPPER_UNIX_H
