/*
 * GuardSarm shared modules utils
 * Copyright (C) 2026 GuardSarm, Inc.
 * Sep 15, 2020.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#include "mapWrapperSafe_test.h"
#include "mapWrapperSafe.h"
#include <chrono>
#include <thread>

void MapWrapperSafeTest::SetUp() {};

void MapWrapperSafeTest::TearDown() {};

TEST_F(MapWrapperSafeTest, insertTest)
{
    Utils::MapWrapperSafe<int, int> mapSafe;
    mapSafe.insert(1, 2);
    EXPECT_EQ(2, mapSafe[1]);
}

TEST_F(MapWrapperSafeTest, eraseTest)
{
    Utils::MapWrapperSafe<int, int> mapSafe;
    mapSafe.insert(1, 2);
    EXPECT_NO_THROW(mapSafe.erase(1));
    EXPECT_EQ(0, mapSafe[1]);
}
