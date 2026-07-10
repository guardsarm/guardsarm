/*
 * GuardSarm shared modules utils
 * Copyright (C) 2026 GuardSarm, Inc.
 * April 11, 2024.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef THREAD_SAFE_MULTI_QUEUE_TESTS_HPP
#define THREAD_SAFE_MULTI_QUEUE_TESTS_HPP

#include "rocksDBQueueCF.hpp"
#include "threadSafeMultiQueue.hpp"
#include "gtest/gtest.h"

class ThreadSafeMultiQueueTest : public ::testing::Test
{
protected:
    ThreadSafeMultiQueueTest() = default;
    virtual ~ThreadSafeMultiQueueTest() = default;

    void SetUp() override;
    void TearDown() override;
};
#endif // THREAD_SAFE_MULTI_QUEUE_TESTS_HPP
