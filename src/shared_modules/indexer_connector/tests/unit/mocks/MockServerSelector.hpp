/*
 * GuardSarm - Indexer connector.
 * Copyright (C) 2026 GuardSarm, Inc.
 * July 7, 2025.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef _MOCK_SERVER_SELECTOR_HPP
#define _MOCK_SERVER_SELECTOR_HPP

#include "secureCommunication.hpp"
#include <gmock/gmock.h>
#include <string>
#include <vector>

/**
 * @brief GMock-based mock for ServerSelector class
 *
 * This mock replaces the custom MockServerSelector class with a proper GMock implementation.
 */
class MockServerSelector
{
public:
    MOCK_METHOD(std::string, getNext, (), ());
    MOCK_METHOD(bool, isAvailable, (), ());

    // Default constructor for GMock
    MockServerSelector() = default;

    // Mock constructor - GMock doesn't mock constructors, so we provide a simple stub
    MockServerSelector(const std::vector<std::string>&, uint32_t, const SecureCommunication&)
    {
        // Mock constructor - does nothing real
    }
};

#endif // _MOCK_SERVER_SELECTOR_HPP
