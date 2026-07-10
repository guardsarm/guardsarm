/*
 * GuardSarm shared modules utils
 * Copyright (C) 2026 GuardSarm, Inc.
 * July 14, 2025.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */
#ifndef _FUNCTION_TRAITS_HPP_
#define _FUNCTION_TRAITS_HPP_

#include <functional>
#include <tuple>

template<typename T>
struct function_traits;

template<typename R, typename... Args>
struct function_traits<std::function<R(Args...)>>
{
    using ReturnType = R;
    using ArgsTuple = std::tuple<Args...>;

    static constexpr size_t arity = sizeof...(Args);

    template<size_t N>
    using ArgType = std::tuple_element_t<N, ArgsTuple>;
};

#endif /* _FUNCTION_TRAITS_HPP_ */
