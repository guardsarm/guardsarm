/*
 * GuardSarm shared modules utils
 * Copyright (C) 2026 GuardSarm, Inc.
 * December 10, 2021.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#include "linuxInfoHelper_test.h"
#include "cmdHelper.h"
#include "linuxInfoHelper.h"

void LinuxInfoHelperTest::SetUp() {};

void LinuxInfoHelperTest::TearDown() {};

TEST_F(LinuxInfoHelperTest, getBootTime)
{
    const auto btimeNumFromFunc {Utils::getBootTime()};

    const auto btimeStr {Utils::exec("grep 'btime' /proc/stat | awk '{print $2}'")};
    const auto btimeNumFromProc {std::stoull(btimeStr)};

    EXPECT_FALSE(btimeNumFromFunc != btimeNumFromProc);
}

TEST_F(LinuxInfoHelperTest, timeTick2unixTime)
{
    const auto startTimeStr {Utils::exec("awk '{print $22}' /proc/1/stat")};
    const auto startTimeNum {std::stoul(startTimeStr)};
    const auto startTimeUnixFunc {Utils::timeTick2unixTime(startTimeNum)};

    const auto startTimeUnixCmd {std::stoul(Utils::exec("date -d \"`ps -o lstart -p 1|tail -n 1`\" +%s"))};

    EXPECT_FALSE(startTimeUnixFunc != startTimeUnixCmd);
}
