/*
 * GuardSarm SysInfo
 * Copyright (C) 2026 GuardSarm, Inc.
 * March 16, 2021.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */
#ifndef _SYSINFO_PACKAGES_BERKELEY_DB_TEST_H
#define _SYSINFO_PACKAGES_BERKELEY_DB_TEST_H

#include "gtest/gtest.h"
#include "gmock/gmock.h"

class RpmLibTest : public ::testing::Test
{
    protected:

        RpmLibTest() = default;
        virtual ~RpmLibTest() = default;

        void SetUp() override;
        void TearDown() override;
};

#endif //_SYSINFO_PACKAGES_BERKELEY_DB_TEST_H
