/*
 * GuardSarm shared modules utils
 * Copyright (C) 2026 GuardSarm, Inc.
 * July 14, 2020.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef THREAD_DISPATCHER_TESTS_H
#define THREAD_DISPATCHER_TESTS_H
#include "gtest/gtest.h"
#include "gmock/gmock.h"

class ThreadDispatcherTest : public ::testing::Test
{
    protected:

        ThreadDispatcherTest() = default;
        virtual ~ThreadDispatcherTest() = default;

        void SetUp() override;
        void TearDown() override;
};
#endif //THREAD_DISPATCHER_TESTS_H