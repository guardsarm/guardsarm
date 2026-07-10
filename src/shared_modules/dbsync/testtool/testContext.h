/*
 * GuardSarm DBSYNC
 * Copyright (C) 2026 GuardSarm, Inc.
 * July 21, 2020.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */
#ifndef _TEST_CONTEXT_H
#define _TEST_CONTEXT_H
#include "dbsync.h"
#include "dbsync.hpp"

struct TestContext
{
    DBSYNC_HANDLE handle;
    TXN_HANDLE txnContext;
    size_t currentId;
    std::string outputPath;
};

#endif //_TEST_CONTEXT_H