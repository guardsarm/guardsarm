/*
 * GuardSarm shared modules utils
 * Copyright (C) 2026 GuardSarm, Inc.
 * October 19, 2020.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */
#ifndef REGISTRY_HELPER_TESTS_H
#define REGISTRY_HELPER_TESTS_H
#include "gtest/gtest.h"

class RegistryUtilsTest : public ::testing::Test
{
    protected:

        RegistryUtilsTest() = default;
        virtual ~RegistryUtilsTest() = default;

        void SetUp() override;
        void TearDown() override;
};
#endif //REGISTRY_HELPER_TESTS_H
