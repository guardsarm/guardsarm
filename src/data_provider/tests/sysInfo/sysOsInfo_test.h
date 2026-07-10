/*
 * GuardSarm SysInfo
 * Copyright (C) 2026 GuardSarm, Inc.
 * November 5, 2020.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */
#ifndef _SYSINFO_OS_TEST_H
#define _SYSINFO_OS_TEST_H
#include "gtest/gtest.h"
#include "gmock/gmock.h"

class SysOsInfoTest : public ::testing::Test
{

    protected:

        SysOsInfoTest() = default;
        virtual ~SysOsInfoTest() = default;

        void SetUp() override;
        void TearDown() override;
};

#endif //_SYSINFO_OS_TEST_H