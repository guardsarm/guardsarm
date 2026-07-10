/*
 * GuardSarm shared modules utils
 * Copyright (C) 2026 GuardSarm, Inc.
 * March 18, 2025.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */
#ifndef REFLECTIVE_JSON_HPP
#define REFLECTIVE_JSON_HPP

#include "gtest/gtest.h"

class ReflectiveJsonTest : public ::testing::Test
{
protected:
    ReflectiveJsonTest() = default;
    virtual ~ReflectiveJsonTest() = default;

    void SetUp() override;
    void TearDown() override;
};
#endif // REFLECTIVE_JSON_HPP
