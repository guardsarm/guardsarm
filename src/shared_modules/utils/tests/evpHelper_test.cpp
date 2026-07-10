/*
 * GuardSarm - Shared Modules utils tests
 * Copyright (C) 2026 GuardSarm, Inc.
 * July 11, 2024.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#include "evpHelper_test.hpp"
#include "evpHelper.hpp"
#include "gtest/gtest.h"

/**
 * @brief Test
 *
 */
TEST_F(EVPHelperTest, ValidEncryptionAndDecryption)
{
    std::vector<char> encryptedData;
    std::string inputData = "This is a test. This is a test.";
    EVPHelper().encryptAES256(inputData, encryptedData);

    std::string decryptedData;

    EXPECT_NO_THROW(EVPHelper().decryptAES256(encryptedData, decryptedData));
    ASSERT_EQ(decryptedData, inputData);
}
