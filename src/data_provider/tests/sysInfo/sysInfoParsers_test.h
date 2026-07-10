/*
 * GuardSarm SysInfo
 * Copyright (C) 2026 GuardSarm, Inc.
 * October 28, 2020.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */
#ifndef _SYSINFO_PARSERS_TEST_H
#define _SYSINFO_PARSERS_TEST_H
#include "gtest/gtest.h"
#include "gmock/gmock.h"

class SysInfoParsersTest : public ::testing::Test
{

    protected:

        SysInfoParsersTest() = default;
        virtual ~SysInfoParsersTest() = default;

        void SetUp() override;
        void TearDown() override;
};

#endif //_SYSINFO_PARSERS_TEST_H