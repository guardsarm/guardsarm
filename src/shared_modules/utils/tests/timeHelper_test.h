/*
 * GuardSarm shared modules utils
 * Copyright (C) 2026 GuardSarm, Inc.
 * December 28, 2020.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef TIME_HELPER_TESTS_H
#define TIME_HELPER_TESTS_H

#include <gtest/gtest.h>

class TimeUtilsTest : public ::testing::Test
{
    protected:

        TimeUtilsTest() = default;
        virtual ~TimeUtilsTest() = default;

        void SetUp() override;
        void TearDown() override;
};

#endif // TIME_HELPER_TESTS_H
