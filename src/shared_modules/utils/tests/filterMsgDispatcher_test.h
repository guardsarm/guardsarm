/*
 * GuardSarm shared modules utils
 * Copyright (C) 2026 GuardSarm, Inc.
 * Sep 1, 2020.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef _FILTER_MSG_DISPATCHER_TESTS_H
#define _FILTER_MSG_DISPATCHER_TESTS_H
#include "gmock/gmock.h"
#include "gtest/gtest.h"

class FilterMsgDispatcherTest : public ::testing::Test
{
protected:
    FilterMsgDispatcherTest() = default;
    virtual ~FilterMsgDispatcherTest() = default;

    void SetUp() override;
    void TearDown() override;
};
#endif //_FILTER_MSG_DISPATCHER_TESTS_H
