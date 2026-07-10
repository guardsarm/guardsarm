/*
 * GuardSarm SysInfo
 * Copyright (C) 2026 GuardSarm, Inc.
 * November 3, 2020.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */
#ifndef _SYSINFO_PORT_TEST_H
#define _SYSINFO_PORT_TEST_H
#include "gtest/gtest.h"
#include "gmock/gmock.h"

class SysInfoPortTest : public ::testing::Test
{

    protected:

        SysInfoPortTest() = default;
        virtual ~SysInfoPortTest() = default;

        void SetUp() override;
        void TearDown() override;
};

#endif //_SYSINFO_PORT_TEST_H