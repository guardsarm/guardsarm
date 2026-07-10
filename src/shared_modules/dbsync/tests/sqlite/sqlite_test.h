/*
 * GuardSarm DBSYNC
 * Copyright (C) 2026 GuardSarm, Inc.
 * June 20, 2020.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */
#ifndef _SQLITE_TEST_H
#define _SQLITE_TEST_H
#include "gtest/gtest.h"
#include "gmock/gmock.h"

class SQLiteTest : public ::testing::Test
{

    protected:

        SQLiteTest() = default;
        virtual ~SQLiteTest() = default;

        void SetUp() override;
        void TearDown() override;
};

#endif //_SQLITE_TEST_H