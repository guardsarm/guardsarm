/*
 * GuardSarm shared modules utils
 * Copyright (C) 2026 GuardSarm, Inc.
 * Jun 4, 2023.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef _ROCKSDB_SAFEQUEUE_TEST_HPP
#define _ROCKSDB_SAFEQUEUE_TEST_HPP

#include "rocksDBQueue.hpp"
#include "threadSafeQueue.h"
#include <gtest/gtest.h>
#include <memory>
#include <thread>

class RocksDBSafeQueueTest : public ::testing::Test
{
protected:
    RocksDBSafeQueueTest() = default;
    ~RocksDBSafeQueueTest() override = default;
    std::unique_ptr<Utils::SafeQueue<std::string, RocksDBQueue<std::string>>> queue;
    //(RocksDBQueue<std::string>("test.db"));
    void SetUp() override;
    void TearDown() override;
};
#endif //_ROCKSDB_SAFEQUEUE_TEST_HPP
