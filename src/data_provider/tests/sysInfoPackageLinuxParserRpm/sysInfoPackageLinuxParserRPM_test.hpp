/*
 * GuardSarm SysInfo
 * Copyright (C) 2026 GuardSarm, Inc.
 * December 17, 2021.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */
#ifndef _SYSINFO_PACKAGES_LINUX_PARSER_RPM_TEST_H
#define _SYSINFO_PACKAGES_LINUX_PARSER_RPM_TEST_H

#include "gtest/gtest.h"
#include "gmock/gmock.h"

class SysInfoPackagesLinuxParserRPMTest : public ::testing::Test
{
    protected:

        SysInfoPackagesLinuxParserRPMTest() = default;
        virtual ~SysInfoPackagesLinuxParserRPMTest() = default;

        void SetUp() override;
        void TearDown() override;
};

#endif //_SYSINFO_PACKAGES_LINUX_PARSER_RPM_TEST_H
