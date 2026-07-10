/*
 * GuardSarm shared modules utils
 * Copyright (C) 2026 GuardSarm, Inc.
 * December 22, 2023.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef NUMERIC_HELPER_TESTS_H
#define NUMERIC_HELPER_TESTS_H
#include "gtest/gtest.h"

class NumericUtilsTest : public ::testing::Test
{
protected:
    /**
     * @brief Construct a new NumericUtilsTest object
     *
     */
    NumericUtilsTest() = default;

    /**
     * @brief Destroy the NumericUtilsTest object
     *
     */
    virtual ~NumericUtilsTest() = default;

    /**
     * @brief SetUp.
     *
     */
    void SetUp() override;

    /**
     * @brief TearDown.
     *
     */
    void TearDown() override;
};
#endif //NUMERIC_HELPER_TESTS_H