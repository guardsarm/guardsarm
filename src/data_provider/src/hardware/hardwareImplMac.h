/*
 * GuardSarm SYSINFO
 * Copyright (C) 2026 GuardSarm, Inc.
 * May 4, 2023.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef _HARDWARE_IMPL_MAC_H
#define _HARDWARE_IMPL_MAC_H

#include "hardwareInterface.h"
#include "hardwareWrapperInterface.h"

class OSHardwareMac final : public IOSHardware
{
        std::shared_ptr<IOSHardwareWrapper> m_wrapper;
    public:
        explicit OSHardwareMac(const std::shared_ptr<IOSHardwareWrapper>& wrapper)
            : m_wrapper(wrapper)
        { }
        // LCOV_EXCL_START
        ~OSHardwareMac() = default;
        // LCOV_EXCL_STOP

        void buildHardwareData(nlohmann::json& hardware) override
        {
            hardware["serial_number"] = m_wrapper->boardSerial();
            hardware["cpu_name"] = m_wrapper->cpuName();
            hardware["cpu_cores"] = m_wrapper->cpuCores();
            hardware["cpu_speed"] = m_wrapper->cpuMhz();
            hardware["memory_total"] = m_wrapper->ramTotal();
            hardware["memory_free"] = m_wrapper->ramFree();
            hardware["memory_used"] = m_wrapper->ramUsage();
        }
};

class FactoryBSDHardware final
{
    public:
        static std::shared_ptr<IOSHardware>create(const std::shared_ptr<IOSHardwareWrapper>& wrapper)
        {
            return std::make_shared<OSHardwareMac>(wrapper);
        }
};

#endif // _HARDWARE_IMPL_MAC_H
