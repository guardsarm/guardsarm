/*
 * GuardSarm shared modules utils
 * Copyright (C) 2026 GuardSarm, Inc.
 * December 10, 2021.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef LINUXINFO_HELPER_TESTS_H
#define LINUXINFO_HELPER_TESTS_H
#include "gtest/gtest.h"

class LinuxInfoHelperTest : public ::testing::Test
{
    protected:

        LinuxInfoHelperTest() = default;
        virtual ~LinuxInfoHelperTest() = default;

        void SetUp() override;
        void TearDown() override;
};
#endif // LINUXINFO_HELPER_TESTS_H
