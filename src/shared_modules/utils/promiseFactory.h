/*
 * Promise factory
 * Copyright (C) 2026 GuardSarm, Inc.
 * May 4, 2022.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */
#include "abstractWait.h"

#ifndef _PROMISE_FACTORY_HPP
#define _PROMISE_FACTORY_HPP

enum PromiseType
{
    NORMAL,
    SLEEP
};

template<PromiseType osType>
class PromiseFactory final
{
public:
    static std::shared_ptr<IWait> getPromiseObject()
    {
        return std::make_shared<PromiseWaiting>();
    }
};

template<>
class PromiseFactory<PromiseType::SLEEP> final
{
public:
    static std::shared_ptr<IWait> getPromiseObject()
    {
        return std::make_shared<BusyWaiting>();
    }
};

#endif // _PROMISE_FACTORY_HPP
