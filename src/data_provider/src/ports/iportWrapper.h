/*
 * GuardSarm SYSINFO
 * Copyright (C) 2026 GuardSarm, Inc.
 * November 3, 2020.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef _PORT_WRAPPER_H
#define _PORT_WRAPPER_H
#include "iportInterface.h"

enum LinuxPortsFieldsData
{
    ENTRY,
    LOCAL_ADDRESS,
    REMOTE_ADDRESS,
    STATE,
    QUEUE,
    TIMER_ACTIVE,
    RETRANSMITION,
    UID,
    TIMEOUT,
    INODE,
    SIZE_LINUX_PORT_FIELDS
};


class IPortWrapper
{
    public:
        // LCOV_EXCL_START
        virtual ~IPortWrapper() = default;
        // LCOV_EXCL_STOP
        virtual std::string protocol() const = 0;
        virtual std::string localIp() const = 0;
        virtual int32_t localPort() const = 0;
        virtual std::string remoteIP() const = 0;
        virtual int32_t remotePort() const = 0;
        virtual int32_t txQueue() const = 0;
        virtual int32_t rxQueue() const = 0;
        virtual int64_t inode() const = 0;
        virtual std::string state() const = 0;
        virtual int32_t pid() const = 0;
        virtual std::string processName() const = 0;
};
#endif // _PORT_WRAPPER_H
