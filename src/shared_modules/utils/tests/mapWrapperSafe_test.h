/*
 * GuardSarm shared modules utils
 * Copyright (C) 2026 GuardSarm, Inc.
 * Sep 15, 2020.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef MAP_WRAPPER_SAFE_TESTS_H
#define MAP_WRAPPER_SAFE_TESTS_H
#include "gtest/gtest.h"
#include "gmock/gmock.h"

class MapWrapperSafeTest : public ::testing::Test
{
    protected:

        MapWrapperSafeTest() = default;
        virtual ~MapWrapperSafeTest() = default;

        void SetUp() override;
        void TearDown() override;
};
#endif //MAP_WRAPPER_SAFE_TESTS_H