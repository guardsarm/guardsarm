/*
 * GuardSarm content manager - Component Tests
 * Copyright (C) 2026 GuardSarm, Inc.
 * July 26, 2023.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#include "contentManager_test.hpp"
#include "contentManager.hpp"

/*
 * @brief Tests singleton of the ContentModule class
 */
TEST_F(ContentModuleTest, TestSingleton)
{
    auto& contentModule = ContentModule::instance();

    EXPECT_EQ(&contentModule, &ContentModule::instance());
}

/*
 * @brief Tests singleton of the ContentModule class and start method
 */
TEST_F(ContentModuleTest, TestSingletonAndStartMethod)
{
    auto& contentModule = ContentModule::instance();

    EXPECT_EQ(&contentModule, &ContentModule::instance());

    EXPECT_NO_THROW(contentModule.start(nullptr));

    EXPECT_NO_THROW(contentModule.stop());
}
