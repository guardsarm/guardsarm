/*
 * GuardSarm shared modules utils
 * Copyright (C) 2026 GuardSarm, Inc.
 * October 17, 2023.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#include "cacheLRU_test.h"
#include "cacheLRU.hpp"

void CacheLRUTest::SetUp() {};

void CacheLRUTest::TearDown() {};

TEST_F(CacheLRUTest, insertAndHit)
{
    auto cacheMemory = LRUCache<int, int>(10);

    EXPECT_NO_THROW(cacheMemory.insertKey(1, 10));
    EXPECT_EQ(cacheMemory.getValue(1).value(), 10);
}

TEST_F(CacheLRUTest, insertAndMiss)
{
    auto cacheMemory = LRUCache<int, int>(10);

    EXPECT_NO_THROW(cacheMemory.insertKey(10, 10));
    auto result = cacheMemory.getValue(1);

    EXPECT_FALSE(result.has_value());
}
