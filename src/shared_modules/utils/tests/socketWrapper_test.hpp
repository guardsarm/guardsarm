/*
 * GuardSarm shared modules utils
 * Copyright (C) 2026 GuardSarm, Inc.
 * May 1, 2023.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef _SOCKET_WRAPPER_TEST_H
#define _SOCKET_WRAPPER_TEST_H

#include "gtest/gtest.h"

class SocketWrapperTest : public ::testing::Test
{
protected:
    SocketWrapperTest() = default;
    virtual ~SocketWrapperTest() = default;

    void SetUp() override;
    void TearDown() override;
};
#endif //_SOCKET_WRAPPER_TEST_H
