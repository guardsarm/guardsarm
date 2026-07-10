/*
 * GuardSarm content manager - Component Tests
 * Copyright (C) 2026 GuardSarm, Inc.
 * July 26, 2023.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef _CONTENT_REGISTER_TEST_HPP
#define _CONTENT_REGISTER_TEST_HPP

#include "fakes/fakeServer.hpp"
#include "gtest/gtest.h"
#include <external/nlohmann/json.hpp>
#include <memory>

/**
 * @brief Runs component tests for ContentRegister
 */
class ContentRegisterTest : public ::testing::Test
{
protected:
    ContentRegisterTest() = default;
    ~ContentRegisterTest() override = default;

    nlohmann::json m_parameters; ///< Parameters used to create the ContentRegister

    inline static std::unique_ptr<FakeServer> m_spFakeServer; ///< Pointer to FakeServer class

    /**
     * @brief Sets initial conditions for each test case.
     */
    // cppcheck-suppress unusedFunction
    void SetUp() override
    {
        m_parameters = R"(
            {
                "topicName": "content-register-tests",
                "interval": 1,
                "ondemand": false,
                "configData": {
                    "consumerName": "ContentRegisterTest",
                    "contentSource": "cti-offset",
                    "compressionType": "raw",
                    "versionedContent": "false",
                    "deleteDownloadedContent": false,
                    "url": "http://localhost:4444/raw/consumers",
                    "outputFolder": "/tmp/content-register-tests",
                    "contentFileName": "sample.json"
                }
            }
        )"_json;
    }

    /**
     * @brief Creates the fakeServer for the runtime of the test suite
     */
    // cppcheck-suppress unusedFunction
    static void SetUpTestSuite()
    {
        if (!m_spFakeServer)
        {
            m_spFakeServer = std::make_unique<FakeServer>("localhost", 4444);
        }
    }

    /**
     * @brief Resets fakeServer causing the shutdown of the test server.
     */
    // cppcheck-suppress unusedFunction
    static void TearDownTestSuite()
    {
        m_spFakeServer.reset();
    }
};

#endif //_CONTENT_REGISTER_TEST_HPP
