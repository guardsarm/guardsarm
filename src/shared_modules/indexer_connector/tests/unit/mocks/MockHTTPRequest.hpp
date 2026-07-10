/*
 * GuardSarm - Indexer connector.
 * Copyright (C) 2026 GuardSarm, Inc.
 * July 7, 2025.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef _MOCK_HTTP_REQUEST_HPP
#define _MOCK_HTTP_REQUEST_HPP

#include "IURLRequest.hpp"
#include <gmock/gmock.h>
#include <string>
#include <variant>
#include <vector>

// Type aliases to avoid GMock issues with comma in template parameters
using RequestParamsVariant = std::
    variant<TRequestParameters<std::string>, TRequestParameters<nlohmann::json>, TRequestParameters<std::string_view>>;

using PostRequestParametersVariant =
    std::variant<TPostRequestParameters<const std::string&>, TPostRequestParameters<std::string&&>>;

/**
 * @brief GMock-based mock for HTTPRequest class
 *
 * This mock replaces the custom MockHttpRequest class with a proper GMock implementation
 * that provides better test isolation and flexibility.
 */
class MockHTTPRequest : public IURLRequest
{
public:
    MOCK_METHOD(void,
                download,
                (RequestParamsVariant requestParameters,
                 PostRequestParametersVariant postRequestParameters,
                 ConfigurationParameters configurationParameters),
                (override));

    MOCK_METHOD(void,
                post,
                (RequestParamsVariant requestParameters,
                 PostRequestParametersVariant postRequestParameters,
                 ConfigurationParameters configurationParameters),
                (override));

    MOCK_METHOD(void,
                get,
                (RequestParamsVariant requestParameters,
                 PostRequestParametersVariant postRequestParameters,
                 ConfigurationParameters configurationParameters),
                (override));

    MOCK_METHOD(void,
                put,
                (RequestParamsVariant requestParameters,
                 PostRequestParametersVariant postRequestParameters,
                 ConfigurationParameters configurationParameters),
                (override));

    MOCK_METHOD(void,
                patch,
                (RequestParamsVariant requestParameters,
                 PostRequestParametersVariant postRequestParameters,
                 ConfigurationParameters configurationParameters),
                (override));

    MOCK_METHOD(void,
                delete_,
                (RequestParamsVariant requestParameters,
                 PostRequestParametersVariant postRequestParameters,
                 ConfigurationParameters configurationParameters),
                (override));

    // Static instance method required by IndexerConnectorSyncImpl
    static MockHTTPRequest& instance()
    {
        static MockHTTPRequest inst;
        return inst;
    }
};

#endif // _MOCK_HTTP_REQUEST_HPP
