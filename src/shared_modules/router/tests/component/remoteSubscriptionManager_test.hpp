/*
 * GuardSarm router - RemoteSubscriptionManager tests
 * Copyright (C) 2026 GuardSarm, Inc.
 * December 19, 2023.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef _REMOTE_STATE_HELPER_TEST_HPP
#define _REMOTE_STATE_HELPER_TEST_HPP

#include "routerModule.hpp"
#include <gtest/gtest.h>

/**
 * @brief Runs unit tests for RemoteSubscriptionManager class
 */
class RemoteSubscriptionManagerTest : public ::testing::Test
{
protected:
    RemoteSubscriptionManagerTest() = default;
    ~RemoteSubscriptionManagerTest() override = default;

    /**
     * @brief Test setup routine.
     *
     */
    void SetUp() override
    {
        RouterModule::instance().start();
    }

    /**
     * @brief Test teardown routine.
     *
     */
    void TearDown() override
    {
        RouterModule::instance().stop();
    };
};

#endif //_REMOTE_STATE_HELPER_TEST_HPP
