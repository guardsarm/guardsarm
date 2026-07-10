/*
 * GuardSarm Syscheck
 * Copyright (C) 2026 GuardSarm, Inc.
 * November 23, 2021.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef _FIMDB_IMP_TEST_H
#define _FIMDB_IMP_TEST_H

#include "dbItem.hpp"
#include "fimCommonDefs.h"
#include "fimDB.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

class MockDBSyncHandler : public DBSync
{

    public:
        MockDBSyncHandler(const HostType hostType,
                          const DbEngineType dbType,
                          const std::string& path,
                          const std::string& sqlStatement,
                          const DbManagement dbManagement = DbManagement::PERSISTENT)
            : DBSync(hostType, dbType, path, sqlStatement, dbManagement) {};
        ~MockDBSyncHandler() {};
        MOCK_METHOD(void, setTableMaxRow, (const std::string&, const long long), (override));
        MOCK_METHOD(void, insertData, (const nlohmann::json&), (override));
        MOCK_METHOD(void, deleteRows, (const nlohmann::json&), (override));
        MOCK_METHOD(void, syncRow, (const nlohmann::json&, ResultCallbackData), (override));
        MOCK_METHOD(void, selectRows, (const nlohmann::json&, ResultCallbackData), (override));
        MOCK_METHOD(void, closeAndDeleteDatabase, (), (override));
};

class MockFIMDB : public FIMDB
{
    public:
        MockFIMDB() {};
        ~MockFIMDB() {};

        void teardown()
        {
            FIMDB::teardown();
        }
};

class MockLoggingCall
{
    public:
        MOCK_METHOD(void, loggingFunction, (const modules_log_level_t, const std::string&), ());
};

#endif //_FIMDB_IMP_TEST_H
