/*
 * GuardSarm Content Manager - Component Tests
 * Copyright (C) 2026 GuardSarm, Inc.
 * Dec 18, 2023.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef _MOCK_ROUTER_PROVIDER_H
#define _MOCK_ROUTER_PROVIDER_H

#include "iRouterProvider.hpp"
#include <gmock/gmock.h>
#include <string>
#include <vector>

/**
 * @class MockRouterProvider
 *
 * @brief Mock implementation of IRouterProvider.
 *
 */
class MockRouterProvider : public IRouterProvider
{
public:
    /**
     * @brief Mock class constructor.
     *
     */
    MockRouterProvider() = default;

    /**
     * @brief Destroy the Mock Router Provider object
     *
     */
    virtual ~MockRouterProvider() = default;

    /**
     * @brief Mock implementation of function IRouterProvider::stop.
     *
     */
    MOCK_METHOD(void, stop, ());

    /**
     * @brief Mock implementation of function IRouterProvider::start.
     *
     */
    MOCK_METHOD(void, start, ());

    /**
     * @brief Mock implementation of function IRouterProvider::start
     */
    MOCK_METHOD(void, start, (const std::function<void()>& onConnect));

    /**
     * @brief Mock implementation of function IRouterProvider::send.
     *
     */
    MOCK_METHOD(void, send, (const std::vector<char>& data));
};

#endif //_MOCK_ROUTER_PROVIDER_H
