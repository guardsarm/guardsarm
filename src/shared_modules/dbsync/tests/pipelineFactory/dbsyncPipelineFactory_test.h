/*
 * GuardSarm DBSYNC
 * Copyright (C) 2026 GuardSarm, Inc.
 * July 16, 2020.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */
#ifndef DBSYNC_PIPELINE_FACTORY_TESTS_H
#define DBSYNC_PIPELINE_FACTORY_TESTS_H
#include "gtest/gtest.h"
#include "gmock/gmock.h"

class DBSyncPipelineFactoryTest : public ::testing::Test
{
    protected:

        DBSyncPipelineFactoryTest()
            : m_pipelineFactory{DbSync::PipelineFactory::instance()}
            , m_dbHandle{ nullptr }
        {}
        virtual ~DBSyncPipelineFactoryTest() = default;

        void SetUp() override;
        void TearDown() override;
        DbSync::PipelineFactory& m_pipelineFactory;
        DBSYNC_HANDLE m_dbHandle;
};
#endif //DBSYNC_PIPELINE_FACTORY_TESTS_H