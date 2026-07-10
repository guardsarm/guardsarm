/*
 * GuardSarm SYSINFO
 * Copyright (C) 2026 GuardSarm, Inc.
 * May 4, 2023.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef _FACTORY_HARDWARE_FAMILY_CREATOR_H
#define _FACTORY_HARDWARE_FAMILY_CREATOR_H

#include <memory>
#include "json.hpp"
#include "hardwareInterface.h"
#include "hardwareWrapperInterface.h"
#include "hardwareImplMac.h"
#include "sharedDefs.h"

template <OSPlatformType osType>
class FactoryHardwareFamilyCreator final
{
    public:
        static std::shared_ptr<IOSHardware> create(const std::shared_ptr<IOSHardwareWrapper>& /*wrapperInterface*/)
        {
            throw std::runtime_error
            {
                "Error creating network data retriever."
            };
        }
};

template <>
class FactoryHardwareFamilyCreator<OSPlatformType::BSDBASED> final
{
    public:
        static std::shared_ptr<IOSHardware> create(const std::shared_ptr<IOSHardwareWrapper>& wrapperInterface)
        {
            return FactoryBSDHardware::create(wrapperInterface);
        }
};

#endif // _FACTORY_HARDWARE_FAMILY_CREATOR_H
