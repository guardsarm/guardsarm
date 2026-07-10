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

template<typename T>
class Subscriber final : public Observer<T>
{
private:
    std::function<void(T&)> m_callback {};

public:
    explicit Subscriber(const std::function<void(T)>& callback)
        : m_callback {callback}
    {
    }
    virtual ~Subscriber() = default;

    void update(T data) override
    {
        m_callback(data);
    }
};

#endif // _SUBSCRIBER_HPP
