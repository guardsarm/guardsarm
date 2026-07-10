/*
 * GuardSarm DBSYNC
 * Copyright (C) 2026 GuardSarm, Inc.
 * June 11, 2020.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef _DBYSNC_TEST_H
#define _DBYSNC_TEST_H

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "commonDefs.h"

class DBSyncTest : public ::testing::Test
{
    protected:

        DBSyncTest() = default;
        virtual ~DBSyncTest() = default;

        void SetUp() override;
        void TearDown() override;
};

struct DummyContext
{
    DBSYNC_HANDLE handle;
    TXN_HANDLE    txnContext;
};

#endif // _DBYSNC_TEST_H
