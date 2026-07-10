/*
 * GuardSarm shared modules utils
 * Copyright (C) 2026 GuardSarm, Inc.
 * October 19, 2020.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef CMD_HELPER_TESTS_H
#define CMD_HELPER_TESTS_H
#include <gtest/gtest.h>

class CmdUtilsTest : public ::testing::Test
{
    protected:

        CmdUtilsTest() = default;
        virtual ~CmdUtilsTest() = default;

        void SetUp() override;
        void TearDown() override;
};
#endif //CMD_HELPER_TESTS_H
