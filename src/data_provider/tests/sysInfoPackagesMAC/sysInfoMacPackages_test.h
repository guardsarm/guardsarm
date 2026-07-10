/*
 * GuardSarm SysInfo
 * Copyright (C) 2026 GuardSarm, Inc.
 * December 14, 2020.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef _SYSINFO_MAC_PACKAGES_TEST_H
#define _SYSINFO_MAC_PACKAGES_TEST_H
#include "gtest/gtest.h"
#include "gmock/gmock.h"

class SysInfoMacPackagesTest : public ::testing::Test
{

    protected:

        SysInfoMacPackagesTest() = default;
        virtual ~SysInfoMacPackagesTest() = default;

        void SetUp() override;
        void TearDown() override;
};

#endif //_SYSINFO_MAC_PACKAGES_TEST_H