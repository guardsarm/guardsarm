/*
 * GuardSarm shared modules utils
 * Copyright (C) 2026 GuardSarm, Inc.
 * Nov 1, 2023.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef _GUARDSARM_DB_QUERY_BUILDER_TEST_HPP
#define _GUARDSARM_DB_QUERY_BUILDER_TEST_HPP

#include "gtest/gtest.h"

class GuardSarmDBQueryBuilderTest : public ::testing::Test
{
protected:
    GuardSarmDBQueryBuilderTest() = default;
    virtual ~GuardSarmDBQueryBuilderTest() = default;

    void SetUp() override {};
    void TearDown() override {};
};

#endif // _GUARDSARM_DB_QUERY_BUILDER_TEST_HPP
