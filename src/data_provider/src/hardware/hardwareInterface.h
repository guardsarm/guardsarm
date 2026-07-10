/*
 * GuardSarm SYSINFO
 * Copyright (C) 2026 GuardSarm, Inc.
 * May 4, 2023.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef _HARDWARE_INTERFACE_H
#define _HARDWARE_INTERFACE_H

#include "json.hpp"

class IOSHardware
{
    public:
        // LCOV_EXCL_START
        virtual ~IOSHardware() = default;
        // LCOV_EXCL_STOP
        virtual void buildHardwareData(nlohmann::json& hardware) = 0;
};

#endif // _HARDWARE_INTERFACE_H
