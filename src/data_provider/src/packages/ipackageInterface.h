/*
 * GuardSarm SYSINFO
 * Copyright (C) 2026 GuardSarm, Inc.
 * December 14, 2020.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef _PACKAGE_INTERFACE_H
#define _PACKAGE_INTERFACE_H

#include "json.hpp"

class IPackage
{
    public:
        // LCOV_EXCL_START
        virtual ~IPackage() = default;
        // LCOV_EXCL_STOP
        virtual void buildPackageData(nlohmann::json& package) = 0;
};

#endif // _PACKAGE_INTERFACE_H