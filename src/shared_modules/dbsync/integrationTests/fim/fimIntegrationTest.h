/*
 * GuardSarm DBSYNC
 * Copyright (C) 2026 GuardSarm, Inc.
 * August 6, 2020.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef _DBYSNC_FIM_INTEGRATION_TEST_H
#define _DBYSNC_FIM_INTEGRATION_TEST_H

#include "gtest/gtest.h"
#include "gmock/gmock.h"

class DBSyncFimIntegrationTest : public ::testing::Test
{
    protected:

        DBSyncFimIntegrationTest();
        virtual ~DBSyncFimIntegrationTest();

        void SetUp() override;
        void TearDown() override;
        const DBSYNC_HANDLE m_dbHandle;
};

#endif // _DBYSNC_FIM_INTEGRATION_TEST_H
