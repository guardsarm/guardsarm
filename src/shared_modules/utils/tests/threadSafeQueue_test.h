/*
 * GuardSarm shared modules utils
 * Copyright (C) 2026 GuardSarm, Inc.
 * July 14, 2020.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef THREAD_SAFE_QUEUE_TESTS_H
#define THREAD_SAFE_QUEUE_TESTS_H
#include "gtest/gtest.h"

class ThreadSafeQueueTest : public ::testing::Test
{
    protected:

        ThreadSafeQueueTest() = default;
        virtual ~ThreadSafeQueueTest() = default;

        void SetUp() override;
        void TearDown() override;
};
#endif //THREAD_SAFE_QUEUE_TESTS_H