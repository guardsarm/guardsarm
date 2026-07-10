/*
 * GuardSarm SYSINFO
 * Copyright (C) 2026 GuardSarm, Inc.
 * October 24, 2020.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef _NETWORK_FAMILY_DATA_AFACTORY_H
#define _NETWORK_FAMILY_DATA_AFACTORY_H

#include <memory>
#include "json.hpp"
#include "networkInterfaceLinux.h"
#include "networkInterfaceBSD.h"
#include "networkInterfaceWindows.h"
#include "sharedDefs.h"

template <OSPlatformType osType>
class FactoryNetworkFamilyCreator final
{
    public:
        static std::shared_ptr<IOSNetwork> create(const std::shared_ptr<INetworkInterfaceWrapper>& /*interface*/)
        {
            throw std::runtime_error
            {
                "Error creating network data retriever."
            };
        }
};

template <>
class FactoryNetworkFamilyCreator<OSPlatformType::LINUX> final
{
    public:
        static std::shared_ptr<IOSNetwork> create(const std::shared_ptr<INetworkInterfaceWrapper>& interfaceWrapper)
        {
            return FactoryLinuxNetwork::create(interfaceWrapper);
        }
};

template <>
class FactoryNetworkFamilyCreator<OSPlatformType::BSDBASED> final
{
    public:
        static std::shared_ptr<IOSNetwork> create(const std::shared_ptr<INetworkInterfaceWrapper>& interfaceWrapper)
        {
            return FactoryBSDNetwork::create(interfaceWrapper);
        }
};

template <>
class FactoryNetworkFamilyCreator<OSPlatformType::WINDOWS> final
{
    public:
        static std::shared_ptr<IOSNetwork> create(const std::shared_ptr<INetworkInterfaceWrapper>& interfaceWrapper)
        {
            return FactoryWindowsNetwork::create(interfaceWrapper);
        }
};

#endif // _NETWORK_FAMILY_DATA_AFACTORY_H
