/*
 * GuardSarm SysInfo
 * Copyright (C) 2026 GuardSarm, Inc.
 * July 20, 2023.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef _PKGWRAPPER_TEST_H
#define _PKGWRAPPER_TEST_H
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include <string>

class PKGWrapperTest : public ::testing::Test
{
    protected:
        PKGWrapperTest() = default;
        virtual ~PKGWrapperTest() = default;

        void SetUp() override;
        void TearDown() override;

        std::string m_tempDir;
};

#endif //_PKGWRAPPER_TEST_H
