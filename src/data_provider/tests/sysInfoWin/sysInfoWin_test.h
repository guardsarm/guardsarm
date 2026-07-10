/*
 * GuardSarm SysInfo
 * Copyright (C) 2026 GuardSarm, Inc.
 * February 25, 2021.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef _SYSINFO_WIN_TEST_H
#define _SYSINFO_WIN_TEST_H

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "utilsWrapperWin.hpp"

class SysInfoWinTest : public ::testing::Test
{
    protected:

        SysInfoWinTest() = default;
        virtual ~SysInfoWinTest() = default;

        void SetUp() override;
        void TearDown() override;
};


class MockComHelper : public IComHelper
{
    public:
        MOCK_METHOD(HRESULT, CreateWmiLocator, (IWbemLocator*& pLoc), (override));
        MOCK_METHOD(HRESULT, ConnectToWmiServer, (IWbemLocator* pLoc, IWbemServices*& pSvc), (override));
        MOCK_METHOD(HRESULT, SetProxyBlanket, (IWbemServices* pSvc), (override));
        MOCK_METHOD(HRESULT, ExecuteWmiQuery, (IWbemServices* pSvc, IEnumWbemClassObject*& pEnumerator), (override));
        MOCK_METHOD(HRESULT, CreateUpdateSearcher, (IUpdateSearcher*& pUpdateSearcher), (override));
        MOCK_METHOD(HRESULT, GetTotalHistoryCount, (IUpdateSearcher* pUpdateSearcher, LONG& count), (override));
        MOCK_METHOD(HRESULT, QueryHistory, (IUpdateSearcher* pUpdateSearcher, IUpdateHistoryEntryCollection*& pHistory,  LONG& count), (override));
        MOCK_METHOD(HRESULT, GetCount, (IUpdateHistoryEntryCollection* pHistory, LONG& count), (override));
        MOCK_METHOD(HRESULT, GetItem, (IUpdateHistoryEntryCollection* pHistory, LONG index, IUpdateHistoryEntry** pEntry), (override));
        MOCK_METHOD(HRESULT, GetTitle, (IUpdateHistoryEntry* pEntry, BSTR& title), (override));
};


#endif //_SYSINFO_WIN_TEST_H
