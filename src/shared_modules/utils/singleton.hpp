/*
 * GuardSarm Utils - Singleton template
 * Copyright (C) 2026 GuardSarm, Inc.
 * May 20, 2022.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef _SINGLETON_HPP
#define _SINGLETON_HPP

template<typename T>
class Singleton
{
public:
    static T& instance()
    {
        static T s_instance;
        return s_instance;
    }

protected:
    Singleton() = default;
    virtual ~Singleton() = default;
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;
};

#endif // _SINGLETON_HPP
