/*
 * GuardSarm shared modules utils
 * Copyright (C) 2026 GuardSarm, Inc.
 * Sep 1, 2020.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef MSG_DISPATCHER_TESTS_H
#define MSG_DISPATCHER_TESTS_H
#include "gtest/gtest.h"
#include "gmock/gmock.h"

class MsgDispatcherTest : public ::testing::Test
{
    protected:

        MsgDispatcherTest() = default;
        virtual ~MsgDispatcherTest() = default;

        void SetUp() override;
        void TearDown() override;
};
#endif //MSG_DISPATCHER_TESTS_H