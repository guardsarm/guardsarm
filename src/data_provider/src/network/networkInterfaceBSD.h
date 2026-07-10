/*
 * GuardSarm SYSINFO
 * Copyright (C) 2026 GuardSarm, Inc.
 * October 24, 2020.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef _NETWORK_BSD_H
#define _NETWORK_BSD_H

#include "inetworkInterface.h"
#include "inetworkWrapper.h"

class FactoryBSDNetwork
{
    public:
        static std::shared_ptr<IOSNetwork>create(const std::shared_ptr<INetworkInterfaceWrapper>& interfaceWrapper);
};

template <unsigned short osNetworkType>
class BSDNetworkImpl final : public IOSNetwork
{
        const std::shared_ptr<INetworkInterfaceWrapper> m_interfaceAddress;
    public:
        explicit BSDNetworkImpl(const std::shared_ptr<INetworkInterfaceWrapper>& interfaceAddress)
            : m_interfaceAddress(interfaceAddress)
        { }
        void buildNetworkData(nlohmann::json& /*network*/) override
        {
            throw std::runtime_error { "Non implemented specialization." };
        }
};

#endif // _NETWORK_BSD_H
