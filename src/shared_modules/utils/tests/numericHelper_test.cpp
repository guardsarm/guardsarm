/*
 * GuardSarm shared modules utils
 * Copyright (C) 2026 GuardSarm, Inc.
 * December 22, 2023.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#include "numericHelper_test.h"
#include "numericHelper.h"

void NumericUtilsTest::SetUp() {};

void NumericUtilsTest::TearDown() {};

TEST_F(NumericUtilsTest, floatToDoubleRound)
{
    EXPECT_DOUBLE_EQ(0.0, Utils::floatToDoubleRound(0.0f, 1));
    EXPECT_DOUBLE_EQ(0.0, Utils::floatToDoubleRound(0.0f, 2));
    EXPECT_DOUBLE_EQ(1.0, Utils::floatToDoubleRound(1.0f, 1));
    EXPECT_DOUBLE_EQ(1.0, Utils::floatToDoubleRound(1.0f, 2));
    EXPECT_DOUBLE_EQ(1.1, Utils::floatToDoubleRound(1.1f, 1));
    EXPECT_DOUBLE_EQ(1.1, Utils::floatToDoubleRound(1.1f, 2));
    EXPECT_DOUBLE_EQ(4.3, Utils::floatToDoubleRound(4.3f, 1));
    EXPECT_DOUBLE_EQ(4.3, Utils::floatToDoubleRound(4.3f, 2));
    EXPECT_DOUBLE_EQ(7.9, Utils::floatToDoubleRound(7.9f, 2));
    EXPECT_DOUBLE_EQ(7.99, Utils::floatToDoubleRound(7.99f, 2));
    EXPECT_DOUBLE_EQ(8.0, Utils::floatToDoubleRound(7.999f, 2));
    EXPECT_DOUBLE_EQ(7.0, Utils::floatToDoubleRound(7.001f, 2));
}
