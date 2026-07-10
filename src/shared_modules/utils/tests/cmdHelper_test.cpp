/*
 * GuardSarm shared modules utils
 * Copyright (C) 2026 GuardSarm, Inc.
 * October 19, 2020.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#include "cmdHelper_test.h"
#include "cmdHelper.h"

void CmdUtilsTest::SetUp() {};

void CmdUtilsTest::TearDown() {};
#ifdef WIN32
TEST_F(CmdUtilsTest, CmdVersion)
{
    const auto result {Utils::exec("ver")};
    EXPECT_FALSE(result.empty());
}
#else
TEST_F(CmdUtilsTest, CmdUname)
{
    const auto result {Utils::exec("uname")};
    EXPECT_FALSE(result.empty());
}
#endif