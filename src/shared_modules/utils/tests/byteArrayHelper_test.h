/*
 * GuardSarm shared modules utils
 * Copyright (C) 2026 GuardSarm, Inc.
 * March 16, 2021.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef BYTE_ARRAY_TESTS_H
#define BYTE_ARRAY_TESTS_H
#include "gtest/gtest.h"

class ByteArrayHelperTest : public ::testing::Test
{
    protected:

        ByteArrayHelperTest() = default;
        virtual ~ByteArrayHelperTest() = default;

        void SetUp() override;
        void TearDown() override;
};
#endif //BYTE_ARRAY_TESTS_H
