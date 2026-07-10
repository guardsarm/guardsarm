/*
 * GuardSarm shared modules utils
 * Copyright (C) 2026 GuardSarm, Inc.
 * Agoust 11, 2023.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef _GLOB_HELPER_TEST_H
#define _GLOB_HELPER_TEST_H
#include "gtest/gtest.h"

class GlobHelperTest : public ::testing::Test
{
    protected:

        GlobHelperTest() = default;
        virtual ~GlobHelperTest() = default;

        void SetUp() override;
        void TearDown() override;
};
#endif // _GLOB_HELPER_TEST_H
