/*
 * GuardSarm shared modules utils
 * Copyright (C) 2026 GuardSarm, Inc.
 * May 24, 2023.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef _SOCKET_TEST_HPP
#define _SOCKET_TEST_HPP

#include "gtest/gtest.h"

template<typename T>
class SocketTest : public ::testing::Test
{
protected:
    SocketTest() = default;
    virtual ~SocketTest() = default;

    void SetUp() {};
    void TearDown() {};
};
#endif // _SOCKET_TEST_HPP
