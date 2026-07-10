/*
 * GuardSarm shared modules utils
 * Copyright (C) 2026 GuardSarm, Inc.
 * May 4, 2024.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef _ROCKSDB_SAFEQUEUE_PREFIX_TEST_HPP
#define _ROCKSDB_SAFEQUEUE_PREFIX_TEST_HPP

#include "rocksDBQueueCF.hpp"
#include "threadSafeMultiQueue.hpp"
#include <gtest/gtest.h>
#include <memory>

class RocksDBSafeQueuePrefixTest : public ::testing::Test
{
protected:
    RocksDBSafeQueuePrefixTest() = default;
    ~RocksDBSafeQueuePrefixTest() override = default;
    std::unique_ptr<Utils::TSafeMultiQueue<std::string, std::string, RocksDBQueueCF<std::string>>> queue;
    void SetUp() override;
    void TearDown() override;
};
#endif //_ROCKSDB_SAFEQUEUE_PREFIX_TEST_HPP
