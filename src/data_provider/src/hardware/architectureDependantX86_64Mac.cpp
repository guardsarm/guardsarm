/*
 * GuardSarm SYSINFO
 * Copyright (C) 2026 GuardSarm, Inc.
 * May 4, 2023.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#include "hardwareWrapperImplMac.h"

double getMhz(IOsPrimitivesMac* osPrimitives)
{
    constexpr auto MHz{1000000};
    uint64_t cpuHz{0};
    size_t len{sizeof(cpuHz)};
    int ret{osPrimitives->sysctlbyname("hw.cpufrequency", &cpuHz, &len, nullptr, 0)};

    if (ret)
    {
        throw std::system_error
        {
            ret,
            std::system_category(),
            "Error reading cpu frequency."
        };
    }

    return static_cast<double>(cpuHz) / MHz;
}
