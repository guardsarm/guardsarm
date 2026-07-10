/*
 * GuardSarm SYSINFO
 * Copyright (C) 2026 GuardSarm, Inc.
 * November 3, 2020.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef _NETWORK_INTERFACE_WINDOWS_H
#define _NETWORK_INTERFACE_WINDOWS_H

#include "inetworkInterface.h"
#include "inetworkWrapper.h"

class FactoryWindowsNetwork
{
    public:
        static std::shared_ptr<IOSNetwork> create(const std::shared_ptr<INetworkInterfaceWrapper>& interfaceAddress);
};

template <int osNetworkType>
class WindowsNetworkImpl final : public IOSNetwork
{
        std::shared_ptr<INetworkInterfaceWrapper> m_interfaceAddress;
    public:
        explicit WindowsNetworkImpl(const std::shared_ptr<INetworkInterfaceWrapper>& interfaceAddress)
            : m_interfaceAddress(interfaceAddress)
        { }
        // LCOV_EXCL_START
        ~WindowsNetworkImpl() = default;
        // LCOV_EXCL_STOP
        void buildNetworkData(nlohmann::json& /*network*/) override
        {
            throw std::runtime_error { "Non implemented specialization." };
        }
};

#endif // _NETWORK_INTERFACE_WINDOWS_H