/*
 * GuardSarm SysInfo
 * Copyright (C) 2026 GuardSarm, Inc.
 * October 19, 2020.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */
#ifndef _SYSINFO_NETWORK_LINUX_TEST_H
#define _SYSINFO_NETWORK_LINUX_TEST_H
#include "gtest/gtest.h"
#include "gmock/gmock.h"

class SysInfoNetworkLinuxTest : public ::testing::Test
{

    protected:

        SysInfoNetworkLinuxTest() = default;
        virtual ~SysInfoNetworkLinuxTest() = default;

        void SetUp() override;
        void TearDown() override;
};

#endif //_SYSINFO_NETWORK_LINUX_TEST_H