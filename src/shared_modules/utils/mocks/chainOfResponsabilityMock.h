/*
 * GuardSarm databaseFeedManager
 * Copyright (C) 2026 GuardSarm, Inc.
 * January 11, 2024.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */
#ifndef _CHAINOFRESPONSABILITYMOCK_HPP
#define _CHAINOFRESPONSABILITYMOCK_HPP

#include "gmock/gmock.h"
#include <memory>

/**
 * @class MockAbstractHandler
 *
 * @brief Mock class for simulating an AbstractHandler object.
 */
template<typename T>
class MockAbstractHandler
{
public:
    MockAbstractHandler() = default;
    virtual ~MockAbstractHandler() = default;

    /**
     * @brief Mock method for setNext.
     *
     * @note This method is intended for testing purposes and does not perform any real action.
     */
    MOCK_METHOD(std::shared_ptr<MockAbstractHandler<T>>, setNext, (const std::shared_ptr<MockAbstractHandler<T>> requestNext), ());

    /**
     * @brief Mock method for setNext.
     *
     * @note This method is intended for testing purposes and does not perform any real action.
     */
    MOCK_METHOD(std::shared_ptr<MockAbstractHandler<T>>, setLast, (const std::shared_ptr<MockAbstractHandler<T>> requestLast), ());

    /**
     * @brief Mock method for handleRequest.
     *
     * @note This method is intended for testing purposes and does not perform any real action.
     */
    MOCK_METHOD(T, handleRequest, (T data), ());

    /**
     * @brief Mock method for handleRequest.
     *
     * @note This method is intended for testing purposes and does not perform any real action.
     */
    MOCK_METHOD(std::shared_ptr<MockAbstractHandler<T>>, next, (), (const));
};

#endif // _CHAINOFRESPONSABILITYMOCK_HPP
