/*
 * GuardSarm shared modules utils
 * Copyright (C) 2026 GuardSarm, Inc.
 * Dec 24, 2024.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef _ROCKSDB_SAFEQUEUE_TEST_HPP
#define _ROCKSDB_SAFEQUEUE_TEST_HPP

#include "rocksDBQueue.hpp"
#include "threadSafeQueue.h"
#include <gtest/gtest.h>
#include <memory>

constexpr auto TEST_DB = "test.db";

class RocksDBQueueTest : public ::testing::Test
{
protected:
    RocksDBQueueTest() = default;
    ~RocksDBQueueTest() override = default;
    std::unique_ptr<RocksDBQueue<std::string>> queue;
    void SetUp() override;
    void TearDown() override;
};
#endif //_ROCKSDB_SAFEQUEUE_TEST_HPP
