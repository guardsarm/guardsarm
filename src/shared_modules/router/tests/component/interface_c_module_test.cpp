/*
 * GuardSarm router - Interface tests
 * Copyright (C) 2026 GuardSarm, Inc.
 * Apr 29, 2022.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#include "interface_c_module_test.hpp"
#include "router.h"

TEST(RouterModuleCInterfaceTest, TestInitialize)
{
    EXPECT_EQ(router_initialize(
                  [](const modules_log_level_t level, const char* log, const char* tag)
                  {
                      std::ignore = level;
                      std::ignore = log;
                      std::ignore = tag;
                  }),
              0);
}

TEST(RouterModuleCInterfaceTest, TestInitializeAndDestroy)
{
    EXPECT_EQ(router_start(), 0);
    EXPECT_EQ(router_stop(), 0);
}

TEST(RouterModuleCInterfaceTest, TestDoubleInitialize)
{
    EXPECT_EQ(router_start(), 0);
    EXPECT_NE(router_start(), 0);
    EXPECT_EQ(router_stop(), 0);
}

TEST(RouterModuleCInterfaceTest, TestDoubleDestroy)
{
    EXPECT_EQ(router_start(), 0);
    EXPECT_EQ(router_stop(), 0);
    EXPECT_NE(router_stop(), 0);
}
