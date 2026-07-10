/*
 * GuardSarm SysInfo
 * Copyright (C) 2026 GuardSarm, Inc.
 * November 7, 2020.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef _SYSINFO_NETWORK_WINDOWS_TEST_H
#define _SYSINFO_NETWORK_WINDOWS_TEST_H

#include "gtest/gtest.h"
#include "gmock/gmock.h"

class SysInfoNetworkWindowsTest : public ::testing::Test
{
    protected:

        SysInfoNetworkWindowsTest() = default;
        virtual ~SysInfoNetworkWindowsTest() = default;

        void SetUp() override;
        void TearDown() override;
};

#endif //_SYSINFO_NETWORK_WINDOWS_TEST_H