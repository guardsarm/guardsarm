/*
 * GuardSarm shared modules utils
 * Copyright (C) 2026 GuardSarm, Inc.
 * July 16, 2023.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef _MOCKFILESYSTEM_HPP
#define _MOCKFILESYSTEM_HPP

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <filesystem>

template<typename T>
class MockFileSystem
{
public:
    MOCK_METHOD(bool, exists, (const std::filesystem::path&), ());
    MOCK_METHOD(bool, is_regular_file, (const std::filesystem::path&), ());
    MOCK_METHOD(bool, is_directory, (const std::filesystem::path&), ());
    MOCK_METHOD(T, directory_iterator, (const std::filesystem::path&), ());
};

#endif // _MOCKFILESYSTEM_HPP
