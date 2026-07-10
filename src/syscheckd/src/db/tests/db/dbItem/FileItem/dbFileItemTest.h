/*
 * GuardSarm Syscheck
 * Copyright (C) 2026 GuardSarm, Inc.
 * October 5, 2021.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef _FILEITEM_TEST_H
#define _FILEITEM_TEST_H
#include "gmock/gmock.h"
#include "gtest/gtest.h"

class FileItemTest : public testing::Test
{
    protected:
        FileItemTest() = default;
        virtual ~FileItemTest() = default;

        void SetUp() override;
        void TearDown() override;
        fim_entry* fimEntryTest;
};

#endif //_FILEITEM_TEST_H
