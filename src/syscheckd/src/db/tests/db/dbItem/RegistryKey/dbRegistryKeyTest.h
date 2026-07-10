/*
 * GuardSarm Syscheck
 * Copyright (C) 2026 GuardSarm, Inc.
 * October 15, 2021.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef _REGISTRYKEY_TEST_H
#define _REGISTRYKEY_TEST_H
#include "gmock/gmock.h"
#include "gtest/gtest.h"

class RegistryKeyTest : public testing::Test
{
    protected:
        RegistryKeyTest() = default;
        virtual ~RegistryKeyTest() = default;

        void SetUp() override;
        void TearDown() override;
        fim_entry* fimEntryTest;
        const nlohmann::json expectedValue = R"(
            {
                "data":[{"architecture":"[x64]","checksum":"a2fbef8f81af27155dcee5e3927ff6243593b91a","gid":"0","group_":"root",
                "mtime":1578075431, "path":"HKEY_LOCAL_MACHINE\\SOFTWARE","permissions":"-rw-rw-r--",
                "uid":"0", "owner":"fakeUser"}],"table":"registry_key"
            }
        )"_json;

        const nlohmann::json inputJson = R"(
        {
            "checksum":"a2fbef8f81af27155dcee5e3927ff6243593b91a", "gid":"0", "group_":"root", "architecture":1,
            "mtime":1578075431, "path":"HKEY_LOCAL_MACHINE\\SOFTWARE", "permissions":"-rw-rw-r--",
            "uid":"0", "owner":"fakeUser", "version":1, "sync":0
        }
    )"_json;
};

#endif //_REGISTRYKEY_TEST_H
