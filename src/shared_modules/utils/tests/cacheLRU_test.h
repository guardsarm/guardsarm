/*
 * GuardSarm shared modules utils
 * Copyright (C) 2026 GuardSarm, Inc.
 * October 17, 2023.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef CACHE_LRU_TESTS_H
#define CACHE_LRU_TESTS_H
#include "gtest/gtest.h"

class CacheLRUTest : public ::testing::Test
{
    protected:

        CacheLRUTest() = default;
        virtual ~CacheLRUTest() = default;

        void SetUp() override;
        void TearDown() override;
};
#endif //CACHE_LRU_TESTS_H
