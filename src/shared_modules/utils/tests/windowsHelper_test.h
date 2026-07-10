/*
 * GuardSarm shared modules utils
 * Copyright (C) 2026 GuardSarm, Inc.
 * November 10, 2020.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef WINDOWS_HELPER_TEST_H
#define WINDOWS_HELPER_TEST_H
#include "gtest/gtest.h"
#include "gmock/gmock.h"

class WindowsHelperTest : public ::testing::Test
{
    protected:

        WindowsHelperTest() = default;
        virtual ~WindowsHelperTest() = default;

        void SetUp() override;
        void TearDown() override;
};

#endif //WINDOWS_HELPER_TEST_H