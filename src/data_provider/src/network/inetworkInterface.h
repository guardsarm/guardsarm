/*
 * GuardSarm SYSINFO
 * Copyright (C) 2026 GuardSarm, Inc.
 * October 24, 2020.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef _NETWORK_INTERFACE_H
#define _NETWORK_INTERFACE_H

#include "json.hpp"

class IOSNetwork
{
    public:
        // LCOV_EXCL_START
        virtual ~IOSNetwork() = default;
        // LCOV_EXCL_STOP
        virtual void buildNetworkData(nlohmann::json& network) = 0;
};


struct LinkStats
{
    unsigned int rxPackets;    /* total packets received */
    unsigned int txPackets;    /* total packets transmitted */
    unsigned int rxBytes;      /* total bytes received */
    unsigned int txBytes;      /* total bytes transmitted */
    unsigned int rxErrors;     /* bad packets received */
    unsigned int txErrors;     /* packet transmit problems */
    unsigned int rxDropped;    /* no space in linux buffers */
    unsigned int txDropped;    /* no space available in linux */
};

#endif // _NETWORK_INTERFACE_H