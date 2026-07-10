/*
 * GuardSarm DBSYNC
 * Copyright (C) 2026 GuardSarm, Inc.
 * June 16, 2020.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef _MOCKSQLITEFACTORY_TEST_H
#define _MOCKSQLITEFACTORY_TEST_H

#include <gmock/gmock.h>
#include <string>
#include "sqlite/sqlite_wrapper_factory.h"

class MockSQLiteFactory : public ISQLiteFactory
{
    public:
        MOCK_METHOD(std::shared_ptr<SQLite::IConnection>,
                    createConnection,
                    (const std::string& path),
                    (override));
        MOCK_METHOD(std::unique_ptr<SQLite::ITransaction>,
                    createTransaction,
                    (std::shared_ptr<SQLite::IConnection>& connection),
                    (override));
        MOCK_METHOD(std::unique_ptr<SQLite::IStatement>,
                    createStatement,
                    (std::shared_ptr<SQLite::IConnection>& connection,
                     const std::string& query), (override));
};

#endif //_MOCKSQLITEFACTORY_TEST_H