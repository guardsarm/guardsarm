/*
 * GuardSarm shared modules utils
 * Copyright (C) 2026 GuardSarm, Inc.
 * September 27, 2022.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef _CUSTOM_DELETER_HPP
#define _CUSTOM_DELETER_HPP

template<typename F, F func>
struct CustomDeleter
{
    template<typename T>
    constexpr void operator()(T* arg) const
    {
        func(arg);
    }
};

#endif // _CUSTOM_DELETER_HPP
