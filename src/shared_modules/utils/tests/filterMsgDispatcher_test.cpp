/*
 * GuardSarm shared modules utils
 * Copyright (C) 2026 GuardSarm, Inc.
 * Sep 1, 2020.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#include "filterMsgDispatcher_test.h"
#include "filterMsgDispatcher.hpp"
#include <chrono>
#include <thread>

void FilterMsgDispatcherTest::SetUp() {};
void FilterMsgDispatcherTest::TearDown() {};

using namespace Utils;
using Value = std::string;
using TestMsgDispatcher = FilterMsgDispatcher<Value>;

class CallbackWrapper
{
public:
    CallbackWrapper() = default;
    ~CallbackWrapper() = default;
    MOCK_METHOD(void, callback, (const Value&));
};

TEST_F(FilterMsgDispatcherTest, rWithoutFilterCallback)
{
    CallbackWrapper callbackWrapper;
    TestMsgDispatcher dispatcher([&](const Value& value) { callbackWrapper.callback(value); }, nullptr);

    EXPECT_CALL(callbackWrapper, callback("1")).Times(1);
    EXPECT_CALL(callbackWrapper, callback("2")).Times(1);
    EXPECT_CALL(callbackWrapper, callback("3")).Times(1);

    dispatcher.push("1");
    dispatcher.push("2");
    dispatcher.push("3");
    dispatcher.rundown();
    EXPECT_TRUE(dispatcher.cancelled());
    EXPECT_EQ(0ul, dispatcher.size());
}

TEST_F(FilterMsgDispatcherTest, WithFilterCallback)
{
    CallbackWrapper callbackWrapper;
    TestMsgDispatcher dispatcher([&](const Value& value) { callbackWrapper.callback(value); },
                                 [](const Value& value) { return value == "2"; });

    EXPECT_CALL(callbackWrapper, callback("1")).Times(0);
    EXPECT_CALL(callbackWrapper, callback("2")).Times(1);
    EXPECT_CALL(callbackWrapper, callback("3")).Times(0);

    dispatcher.push("1");
    dispatcher.push("2");
    dispatcher.push("3");
    dispatcher.rundown();
    EXPECT_TRUE(dispatcher.cancelled());
    EXPECT_EQ(0ul, dispatcher.size());
}

TEST_F(FilterMsgDispatcherTest, NullCallback)
{
    EXPECT_ANY_THROW(TestMsgDispatcher dispatcher(nullptr));
}
