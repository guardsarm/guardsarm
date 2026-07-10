/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <cstdlib>

// Global test environment to ensure var/run directory exists
class GlobalTestEnvironment : public ::testing::Environment
{
    public:
        void SetUp() override
        {
            // Create var/run directory before any tests run
            // This ensures the metadata provider shared memory file can be created
            std::system("mkdir -p var/run 2>/dev/null");
        }
};

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::AddGlobalTestEnvironment(new GlobalTestEnvironment);
    return RUN_ALL_TESTS();
}
