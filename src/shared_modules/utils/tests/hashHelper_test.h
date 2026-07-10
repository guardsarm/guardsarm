/*
 * GuardSarm shared modules utils
 * Copyright (C) 2026 GuardSarm, Inc.
 * Sep 8, 2020.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef HASH_HELPER_TESTS_H
#define HASH_HELPER_TESTS_H
#include "gtest/gtest.h"
#include "gmock/gmock.h"

class HashHelperTest : public ::testing::Test
{
    protected:

        HashHelperTest() = default;
        virtual ~HashHelperTest() = default;

        void SetUp() override;
        void TearDown() override;
};
#endif //HASH_HELPER_TESTS_H