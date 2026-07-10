/*
 * GuardSarm SysInfo
 * Copyright (C) 2026 GuardSarm, Inc.
 * July 16, 2023.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef _NPMTEST_HPP
#define _NPMTEST_HPP

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "packagesNPM.hpp"
#include "json.hpp"
#include <filesystem>
#include <ifilesystem_wrapper.hpp>
#include <mock_filesystem_wrapper.hpp>
#include <MockJsonIO.hpp>

class NPMTest : public ::testing::Test
{
    protected:
        MockFileSystemWrapper* mockFileSystem; // Raw pointer - npm will own it
        std::unique_ptr<NPM<MockJsonIO>> npm;

        void SetUp() override
        {
            mockFileSystem = new MockFileSystemWrapper();
            npm = std::make_unique<NPM<MockJsonIO>>(std::unique_ptr<IFileSystemWrapper>(mockFileSystem));
        }

        void TearDown() override
        {
            npm.reset(); // This will delete mockFileSystem
        }
};



#endif // _NPMTEST_HPP
