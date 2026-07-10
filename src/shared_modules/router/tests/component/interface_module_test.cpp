/*
 * GuardSarm router - Interface tests
 * Copyright (C) 2026 GuardSarm, Inc.
 * Apr 29, 2022.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#include "interface_module_test.hpp"
#include "routerModule.hpp"

TEST(RouterModuleInterfaceTest, TestSingleton)
{
    auto& routerModule = RouterModule::instance();
    EXPECT_EQ(&routerModule, &RouterModule::instance());
}

TEST(RouterModuleInterfaceTest, TestInitializeAndDestroy)
{
    auto& routerModule = RouterModule::instance();
    EXPECT_NO_THROW({
        routerModule.start();
        routerModule.stop();
    });
}

TEST(RouterModuleInterfaceTest, TestDoubleInitialize)
{
    auto& routerModule = RouterModule::instance();
    EXPECT_NO_THROW({ routerModule.start(); });

    EXPECT_THROW({ routerModule.start(); }, std::runtime_error);

    EXPECT_NO_THROW({ routerModule.stop(); });
}

TEST(RouterModuleInterfaceTest, TestDoubleDestroy)
{
    auto& routerModule = RouterModule::instance();
    EXPECT_NO_THROW({
        routerModule.start();
        routerModule.stop();
    });

    EXPECT_THROW({ routerModule.stop(); }, std::runtime_error);
}
