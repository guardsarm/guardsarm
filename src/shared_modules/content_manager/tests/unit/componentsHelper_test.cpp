/*
 * GuardSarm Content Manager - Unit Tests
 * Copyright (C) 2026 GuardSarm, Inc.
 * Nov 29, 2023.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#include "componentsHelper_test.hpp"
#include "componentsHelper.hpp"
#include "json.hpp"
#include "updaterContext.hpp"
#include "gtest/gtest.h"

/**
 * @brief Tests the pushComponentStatus() method with a clean context.
 *
 */
TEST_F(ComponentsHelperTest, PushComponentStatusCleanContext)
{
    const auto expectedData = R"(
        {
            "type": "raw",
            "offset": 0,
            "paths": [],
            "stageStatus":
            [
                {
                    "stage": "ComponentName",
                    "status": "ok"
                }
            ]
        }
    )"_json;

    UpdaterContext context;
    ASSERT_NO_THROW(Components::pushStatus("ComponentName", Components::Status::STATUS_OK, context));
    EXPECT_EQ(expectedData, context.data);
}

/**
 * @brief Tests the pushComponentStatus() method with a clean context and two pushes in a row each with a different
 * status.
 *
 */
TEST_F(ComponentsHelperTest, PushComponentStatusCleanContextOkAndFailStatus)
{
    const auto expectedData = R"(
        {
            "type": "raw",
            "offset": 0,
            "paths": [],
            "stageStatus":
            [
                {
                    "stage": "ComponentName",
                    "status": "ok"
                },
                {
                    "stage": "ComponentName",
                    "status": "fail"
                }
            ]
        }
    )"_json;

    UpdaterContext context;
    ASSERT_NO_THROW(Components::pushStatus("ComponentName", Components::Status::STATUS_OK, context));
    ASSERT_NO_THROW(Components::pushStatus("ComponentName", Components::Status::STATUS_FAIL, context));
    EXPECT_EQ(expectedData, context.data);
}

/**
 * @brief Tests the pushComponentStatus() method with a context containing paths.
 *
 */
TEST_F(ComponentsHelperTest, PushComponentStatusWithPaths)
{
    const auto expectedData = R"(
        {
            "type": "raw",
            "offset": 0,
            "paths": [
                "/tmp/file.txt"
            ],
            "stageStatus":
            [
                {
                    "stage": "ComponentName",
                    "status": "ok"
                }
            ]
        }
    )"_json;

    UpdaterContext context;
    context.data.at("paths").push_back("/tmp/file.txt");

    ASSERT_NO_THROW(Components::pushStatus("ComponentName", Components::Status::STATUS_OK, context));
    EXPECT_EQ(expectedData, context.data);
}

/**
 * @brief Tests the pushComponentStatus() method with a context containing stage status.
 *
 */
TEST_F(ComponentsHelperTest, PushComponentStatusWithStageStatus)
{
    const auto expectedData = R"(
        {
            "type": "raw",
            "offset": 0,
            "paths": [],
            "stageStatus":
            [
                {
                    "stage": "SomeOtherComponentName",
                    "status": "ok"
                },
                {
                    "stage": "ComponentName",
                    "status": "ok"
                }
            ]
        }
    )"_json;

    UpdaterContext context;
    context.data.at("stageStatus").push_back(R"({"stage":"SomeOtherComponentName","status":"ok"})"_json);

    ASSERT_NO_THROW(Components::pushStatus("ComponentName", Components::Status::STATUS_OK, context));
    EXPECT_EQ(expectedData, context.data);
}

/**
 * @brief Tests the pushComponentStatus() method with a context containing paths and stage status.
 *
 */
TEST_F(ComponentsHelperTest, PushComponentStatusWithStageStatusAndPath)
{
    const auto expectedData = R"(
        {
            "type": "raw",
            "offset": 0,
            "paths": [
                "/tmp/file.txt"
            ],
            "stageStatus":
            [
                {
                    "stage": "SomeOtherComponentName",
                    "status": "ok"
                },
                {
                    "stage": "ComponentName",
                    "status": "ok"
                }
            ]
        }
    )"_json;

    UpdaterContext context;
    context.data.at("paths").push_back("/tmp/file.txt");
    context.data.at("stageStatus").push_back(R"({"stage":"SomeOtherComponentName","status":"ok"})"_json);

    ASSERT_NO_THROW(Components::pushStatus("ComponentName", Components::Status::STATUS_OK, context));
    EXPECT_EQ(expectedData, context.data);
}

/**
 * @brief Tests the pushComponentStatus() method with an empty component name.
 *
 */
TEST_F(ComponentsHelperTest, PushComponentStatusWithEmptyComponentName)
{
    const auto expectedData = R"(
        {
            "type": "raw",
            "offset": 0,
            "paths": [],
            "stageStatus": []
        }
    )"_json;

    UpdaterContext context;
    ASSERT_THROW(Components::pushStatus("", Components::Status::STATUS_OK, context), std::runtime_error);
    EXPECT_EQ(expectedData, context.data);
}
