/*
 * GuardSarm shared modules utils
 * Copyright (C) 2026 GuardSarm, Inc.
 * July 16, 2023.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */
#ifndef _MOCKFILEIO_HPP
#define _MOCKFILEIO_HPP

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <filesystem>

class MockFileIO
{
public:
    MOCK_METHOD(void,
                readLineByLine,
                (const std::filesystem::path&, const std::function<bool(const std::string&)>&),
                ());
};

#endif // _MOCKFILEIO_HPP
