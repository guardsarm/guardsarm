/*
 * GuardSarm router
 * Copyright (C) 2026 GuardSarm, Inc.
 * Jun 26, 2023.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef _IROUTER_PROVIDER_HPP
#define _IROUTER_PROVIDER_HPP

#include <functional>
#include <vector>

/**
 * @brief IRouterProvider
 *
 */
class IRouterProvider
{
public:
    /**
     * @brief Stops the local or remote provider.
     */
    virtual void stop() = 0;

    /**
     * @brief Starts the local or remote provider.
     */
    virtual void start() = 0;

    /**
     * @brief Starts the local or remote provider.
     * @param onConnect Callback to be called when the provider is connected.
     */
    virtual void start(const std::function<void()>& onConnect) = 0;

    /**
     * @brief Sends the data to the provider.
     *
     * @param data Data to be sent
     */
    virtual void send(const std::vector<char>& data) = 0;

    /**
     * @brief Destroy the IRouterProvider object
     *
     */
    // LCOV_EXCL_START
    virtual ~IRouterProvider() = default;
    // LCOV_EXCL_STOP
};

#endif //_IROUTER_PROVIDER_HPP
