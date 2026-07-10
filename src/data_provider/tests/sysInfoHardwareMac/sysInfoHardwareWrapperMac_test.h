/*
 * GuardSarm SysInfo
 * Copyright (C) 2026 GuardSarm, Inc.
 * May 18, 2023.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef _SYSINFO_HARDWARE_WRAPPER_MAC_TEST_H
#define _SYSINFO_HARDWARE_WRAPPER_MAC_TEST_H

#include "gtest/gtest.h"
#include "gmock/gmock.h"

class SysInfoHardwareWrapperMacTest : public ::testing::Test
{
    protected:
        SysInfoHardwareWrapperMacTest() = default;
        virtual ~SysInfoHardwareWrapperMacTest() = default;

        void SetUp() override;
        void TearDown() override;
};

#endif //_SYSINFO_HARDWARE_WRAPPER_MAC_TEST_H
