/*
 * GuardSarm shared modules utils
 * Copyright (C) 2026 GuardSarm, Inc.
 * July 16, 2023.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */
#ifndef _MOCKJSONIO_HPP
#define _MOCKJSONIO_HPP

#include "json.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <filesystem>

class MockJsonIO
{
public:
    MOCK_METHOD(nlohmann::json, readJson, (const std::filesystem::path&), ());
};

#endif // _MOCKJSONIO_HPP
