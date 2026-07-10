/*
 * GuardSarm router - Interface tests
 * Copyright (C) 2026 GuardSarm, Inc.
 * Apr 29, 2022.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef _ROUTER_INTERFACE_TESTS_HPP
#define _ROUTER_INTERFACE_TESTS_HPP

#include <gtest/gtest.h>

/**
 * @brief RouterInterfaceTest class.
 *
 */
class RouterInterfaceTest : public ::testing::Test
{
protected:
    RouterInterfaceTest() = default;
    ~RouterInterfaceTest() override = default;

    /**
     * @brief Test setup routine.
     *
     */
    void SetUp() override;

    /**
     * @brief Test teardown routine.
     *
     */
    void TearDown() override;
};

/**
 * @brief RouterInterfaceTestNoBroker class with no SetUp.
 *
 */
class RouterInterfaceTestNoBroker : public ::testing::Test
{
protected:
    RouterInterfaceTestNoBroker() = default;
    ~RouterInterfaceTestNoBroker() override = default;

    /**
     * @brief Test setup routine.
     *
     */
    void SetUp() override {};

    /**
     * @brief Test teardown routine.
     *
     */
    void TearDown() override {};
};

#endif //_ROUTER_INTERFACE_TESTS_HPP
