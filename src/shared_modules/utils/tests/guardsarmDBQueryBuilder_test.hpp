/*
 * GuardSarm shared modules utils
 * Copyright (C) 2015, Wazuh Inc.
 * Copyright (C) 2026, GuardSarm.
 * Nov 1, 2023.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation.
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
