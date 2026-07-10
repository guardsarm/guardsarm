/*
 * GuardSarm shared modules utils
 * Copyright (C) 2026 GuardSarm, Inc.
 * July 14, 2020.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef PIPELINE_NODE_TESTS_H
#define PIPELINE_NODE_TESTS_H
#include "gtest/gtest.h"
#include "gmock/gmock.h"

class PipelineNodesTest : public ::testing::Test
{
    protected:

        PipelineNodesTest() = default;
        virtual ~PipelineNodesTest() = default;

        void SetUp() override;
        void TearDown() override;
};

#endif //PIPELINE_NODE_TESTS_H