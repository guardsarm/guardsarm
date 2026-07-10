/*
 * GuardSarm shared modules utils
 * Copyright (C) 2026 GuardSarm, Inc.
 * July 14, 2020.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef STRING_HELPER_TESTS_H
#define STRING_HELPER_TESTS_H
#include "gtest/gtest.h"

class StringUtilsTest : public ::testing::Test
{
    protected:

        StringUtilsTest() = default;
        virtual ~StringUtilsTest() = default;

        void SetUp() override;
        void TearDown() override;
};
#endif //STRING_HELPER_TESTS_H