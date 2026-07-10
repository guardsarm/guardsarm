/*
 * GuardSarm router - RemoteSubscriptionManager tests
 * Copyright (C) 2026 GuardSarm, Inc.
 * December 19, 2023.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#include "remoteSubscriptionManager_test.hpp"
#include "src/remoteSubscriptionManager.hpp"
#include <external/nlohmann/json.hpp>

/**
 * @brief Tests sendInitProviderMessage method.
 *
 */
TEST_F(RemoteSubscriptionManagerTest, sendInitProviderMessageTest)
{
    auto endpointName {"test-remote"};
    RemoteSubscriptionManager remoteSubscriptionManager {};
    std::promise<void> promise;
    EXPECT_NO_THROW(remoteSubscriptionManager.sendInitProviderMessage(endpointName, [&]() { promise.set_value(); }));

    if (promise.get_future().wait_for(std::chrono::seconds(5)) == std::future_status::timeout)
    {
        FAIL() << "Timeout waiting for provider initialization";
    }
}
