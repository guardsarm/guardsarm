/*
 * GuardSarm SysInfo
 * Copyright (C) 2026 GuardSarm, Inc.
 * October 7, 2020.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */
#include <sys/types.h>
#include <sys/sysctl.h>
#include "sysInfo.hpp"
#include "cmdHelper.h"
#include "stringHelper.h"
#include "networkUnixHelper.h"
#include "network/networkBSDWrapper.h"
#include "network/networkFamilyDataAFactory.h"

nlohmann::json SysInfo::getNetworks() const
{
    nlohmann::json networks;

    std::unique_ptr<ifaddrs, Utils::IfAddressSmartDeleter> interfacesAddress;
    std::map<std::string, std::vector<ifaddrs*>> networkInterfaces;
    Utils::NetworkUnixHelper::getNetworks(interfacesAddress, networkInterfaces);

    for (const auto& interface : networkInterfaces)
    {
        nlohmann::json ifaddr {};

        for (const auto addr : interface.second)
        {
            const auto networkInterfacePtr { FactoryNetworkFamilyCreator<OSPlatformType::BSDBASED>::create(std::make_shared<NetworkBSDInterface>(addr)) };

            if (networkInterfacePtr)
            {
                networkInterfacePtr->buildNetworkData(ifaddr);
            }
        }

        networks["iface"].push_back(ifaddr);
    }

    return networks;
}

