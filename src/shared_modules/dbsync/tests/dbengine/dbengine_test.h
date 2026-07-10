/*
 * GuardSarm DBSYNC
 * Copyright (C) 2026 GuardSarm, Inc.
 * July 16, 2020.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef _DBENGINE_TEST_H
#define _DBENGINE_TEST_H
#include "gtest/gtest.h"

class DBEngineTest : public ::testing::Test
{
    protected:

        DBEngineTest() = default;
        virtual ~DBEngineTest() = default;
};

#endif //_DBENGINE_TEST_H