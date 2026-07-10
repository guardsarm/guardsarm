/*
 * GuardSarm shared modules utils
 * Copyright (C) 2026 GuardSarm, Inc.
 * February 17, 2020.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef ENCODING_WINDOWS_HELPER_TEST_H
#define ENCODING_WINDOWS_HELPER_TEST_H
#include "gtest/gtest.h"
#include "gmock/gmock.h"

class EncodingWindowsHelperTest : public ::testing::Test
{
    protected:

        EncodingWindowsHelperTest() = default;
        virtual ~EncodingWindowsHelperTest() = default;

        void SetUp() override;
        void TearDown() override;
};

#endif //ENCODING_WINDOWS_HELPER_TEST_H