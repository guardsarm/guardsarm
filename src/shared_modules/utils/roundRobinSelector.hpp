/*
 * GuardSarm shared modules utils
 * Copyright (C) 2026 GuardSarm, Inc.
 * June 6, 2023.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef _ROUND_ROBIN_SELECTOR_HPP
#define _ROUND_ROBIN_SELECTOR_HPP

#include <atomic>
#include <vector>

template<typename T>
class RoundRobinSelector
{
public:
    explicit RoundRobinSelector(std::vector<T> values)
        : m_values(std::move(values))
    {
    }

    T& getNext()
    {
        if (m_values.empty())
        {
            throw std::runtime_error("No servers available");
        }

        auto idx = m_index.fetch_add(1) % m_values.size();
        return m_values[idx];
    }

private:
    std::vector<T> m_values;
    std::atomic<std::size_t> m_index {0};
};

#endif //_ROUND_ROBIN_SELECTOR_HPP
