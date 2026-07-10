/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#pragma once

#include <string>

#include "json.hpp"

#include <ifilesystem_wrapper.hpp>

class SudoersProvider
{
    public:
        /// Constructor
        explicit SudoersProvider(std::string fileName, std::unique_ptr<IFileSystemWrapper> fileSystemWrapper = nullptr);

        /// Default constructor
        SudoersProvider();

        nlohmann::json collect();

    private:
        void genSudoersFile(const std::string& fileName,
                            unsigned int level,
                            nlohmann::json& results);

        // #if !defined(FREEBSD)
        // const std::string kSudoFile = "/etc/sudoers";
        // #else
        // const std::string kSudoFile = "/usr/local/etc/sudoers";
        // #endif
        std::string m_sudoFile;
        std::unique_ptr<IFileSystemWrapper> m_fileSystemWrapper;
};
