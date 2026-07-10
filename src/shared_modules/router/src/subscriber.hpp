/*
 * GuardSarm router
 * Copyright (C) 2026 GuardSarm, Inc.
 * March 25, 2023.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef _SUBSCRIBER_HPP
#define _SUBSCRIBER_HPP

#include "observer.hpp"
#include <functional>

/**
 * @brief Subscriber class.
 *
 * @tparam T Subscriber data type.
 */
template<typename T>
class Subscriber final : public Observer<T>
{
private:
    std::function<void(T)> m_callback {};

public:
    /**
     * @brief Class constructor.
     *
     * @param callback Update callback.
     * @param observerId Observer ID.
     */
    explicit Subscriber(const std::function<void(T)>& callback, std::string observerId)
        : m_callback {callback}
        , Observer<T>(std::move(observerId))
    {
    }

    /**
     * @brief Destroy the Subscriber object
     *
     */
    // LCOV_EXCL_START
    ~Subscriber() = default;
    // LCOV_EXCL_STOP

    /**
     * @brief Executes update callback.
     *
     * @param data Data to be used on the callback routine.
     */
    void update(T data)
    {
        m_callback(data);
    }
};

#endif // _SUBSCRIBER_HPP
