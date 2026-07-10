/*
 * GuardSarm SYSINFO
 * Copyright (C) 2026 GuardSarm, Inc.
 * November 3, 2020.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef _PORT_INTERFACE_H
#define _PORT_INTERFACE_H

#include <memory>
#include "json.hpp"

class IOSPort
{
    public:
        // LCOV_EXCL_START
        virtual ~IOSPort() = default;
        // LCOV_EXCL_STOP
        virtual void buildPortData(nlohmann::json& port) = 0;
};

#endif // _PORT_INTERFACE_H