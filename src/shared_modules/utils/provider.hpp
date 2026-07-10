/*
 * GuardSarm router
 * Copyright (C) 2026 GuardSarm, Inc.
 * March 25, 2023.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef _PROVIDER_HPP
#define _PROVIDER_HPP

#include "observer.hpp"

template<typename T>
class Provider
{
protected:
    Subject<T> m_subject {};

public:
    virtual ~Provider() = default;
    void addSubscriber(std::shared_ptr<Observer<T>> subscriber)
    {
        m_subject.attach(std::move(subscriber));
    }

    void removeSubscriber(const std::string& observerId)
    {
        m_subject.detach(observerId);
    }

    void call(T data)
    {
        m_subject.setData(data);
    }
};

#endif // _PROVIDER_HPP
