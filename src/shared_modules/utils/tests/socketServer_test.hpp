/*
 * GuardSarm shared modules utils
 * Copyright (C) 2026 GuardSarm, Inc.
 * Oct 22, 2024.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef _SOCKET_SERVER_TEST_HPP
#define _SOCKET_SERVER_TEST_HPP

#include "gtest/gtest.h"

class SocketServerTest : public ::testing::Test
{
protected:
    SocketServerTest() = default;
    virtual ~SocketServerTest() = default;

    void SetUp() override;
    void TearDown() override;
};
#endif //_SOCKET_SERVER_TEST_HPP
