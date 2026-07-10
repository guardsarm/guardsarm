/*
 * GuardSarm shared modules utils
 * Copyright (C) 2026 GuardSarm, Inc.
 * March 16, 2021.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#include "byteArrayHelper_test.h"
#include "byteArrayHelper.h"

void ByteArrayHelperTest::SetUp() {};

void ByteArrayHelperTest::TearDown() {};

constexpr uint8_t bufferBE[] = {0x12, 0x34, 0x56, 0x78};
constexpr uint8_t bufferLE[] = {0x78, 0x56, 0x34, 0x12};
constexpr int32_t result {305419896};

TEST_F(ByteArrayHelperTest, toInt32BE)
{
    EXPECT_EQ(result, Utils::toInt32BE(bufferBE));
}

TEST_F(ByteArrayHelperTest, toInt32LE)
{
    EXPECT_EQ(result, Utils::toInt32LE(bufferLE));
}
