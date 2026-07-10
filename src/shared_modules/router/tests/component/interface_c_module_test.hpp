/*
 * GuardSarm router - Interface tests
 * Copyright (C) 2026 GuardSarm, Inc.
 * Apr 29, 2022.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef _INTERFACE_C_MODULE_TEST_HPP
#define _INTERFACE_C_MODULE_TEST_HPP

#include <gtest/gtest.h>

/**
 * @brief RouterModuleCInterfaceTest class.
 *
 */
class RouterModuleCInterfaceTest : public ::testing::Test
{
protected:
    RouterModuleCInterfaceTest() = default;
    ~RouterModuleCInterfaceTest() override = default;

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
#endif //_INTERFACE_C_MODULE_TEST_HPP
