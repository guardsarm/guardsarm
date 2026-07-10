/*
 * GuardSarm shared modules utils
 * Copyright (C) 2026 GuardSarm, Inc.
 * Jun 4, 2023.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef _THREAD_EVENT_DISPATCHER_TEST_HPP
#define _THREAD_EVENT_DISPATCHER_TEST_HPP

#include "rocksDBQueue.hpp"
#include "threadSafeQueue.h"
#include <gtest/gtest.h>
#include <memory>
#include <thread>

class ThreadEventDispatcherTest : public ::testing::Test
{
protected:
    ThreadEventDispatcherTest() = default;
    ~ThreadEventDispatcherTest() override = default;
    void SetUp() override;
    void TearDown() override;
};
#endif //_THREAD_EVENT_DISPATCHER_TEST_HPP
