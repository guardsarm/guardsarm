/*
 * GuardSarm Syscheck - Test tool
 * Copyright (C) 2026 GuardSarm, Inc.
 * January 21, 2022.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */
#ifndef _TEST_CONTEXT_H
#define _TEST_CONTEXT_H
#include <mutex>
#include "dbsync.h"
#include "dbsync.hpp"

static const std::map<ReturnTypeCallback, std::string> RETURN_TYPE_OPERATION =
{
    { MODIFIED, "MODIFIED" },
    { DELETED,  "DELETED"  },
    { INSERTED, "INSERTED" },
    { MAX_ROWS, "MAX_ROWS" },
    { DB_ERROR, "DB_ERROR" },
    { SELECTED, "SELECTED" },
    { GENERIC,  "GENERIC"  }
};

struct TestContext
{
    DBSYNC_HANDLE handle;
    std::unique_ptr<DBSyncTxn> txn;
    std::mutex txn_callback_mutex;
    size_t currentId;
    std::string outputPath;

};

#endif //_TEST_CONTEXT_H
