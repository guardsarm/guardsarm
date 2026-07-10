/*
 * GuardSarm SyscollectorNormalizer
 * Copyright (C) 2026 GuardSarm, Inc.
 * January 12, 2021.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */
#ifndef _SYS_NORMALIZER_TEST_H
#define _SYS_NORMALIZER_TEST_H
#include "gtest/gtest.h"
#include "gmock/gmock.h"

class SysNormalizerTest : public ::testing::Test
{
    protected:

        SysNormalizerTest() = default;
        virtual ~SysNormalizerTest() = default;

        void SetUp() override;
        void TearDown() override;
};

#endif //_SYS_NORMALIZER_TEST_H