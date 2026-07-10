/*
 * GuardSarm SysInfo
 * Copyright (C) 2026 GuardSarm, Inc.
 * January 28, 2021.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */
#ifndef _SYSINFO_PACKAGES_LINUX_HELPER_TEST_H
#define _SYSINFO_PACKAGES_LINUX_HELPER_TEST_H

#include "gtest/gtest.h"
#include "gmock/gmock.h"

class SysInfoPackagesLinuxHelperTest : public ::testing::Test
{
    protected:

        SysInfoPackagesLinuxHelperTest() = default;
        virtual ~SysInfoPackagesLinuxHelperTest() = default;

        void SetUp() override;
        void TearDown() override;
};

#endif //_SYSINFO_PACKAGES_LINUX_HELPER_TEST_H