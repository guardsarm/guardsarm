/*
 * GuardSarm SyscollectorImp
 * Copyright (C) 2026 GuardSarm, Inc.
 * November 9, 2020.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */
#ifndef _SYSCOLLECTOR_IMP_TEST_H
#define _SYSCOLLECTOR_IMP_TEST_H
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include <memory>

// Forward declaration
class MockSchemaValidatorEngine;

class SyscollectorImpTest : public ::testing::Test
{
    protected:

        SyscollectorImpTest() = default;
        virtual ~SyscollectorImpTest() = default;

        void SetUp() override;
        void TearDown() override;

        // Store mock validator to keep it alive during test execution
        std::shared_ptr<MockSchemaValidatorEngine> m_mockValidator;
};

#endif //_SYSCOLLECTOR_IMP_TEST_H