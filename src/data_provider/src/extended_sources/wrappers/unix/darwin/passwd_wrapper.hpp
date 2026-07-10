/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#pragma once

#include "ipasswd_wrapper.hpp"

/// @brief Wrapper class for macOS-specific passwd database access.
///
/// Encapsulates system calls used to retrieve user information on Darwin/macOS,
/// allowing for dependency injection and easier testing.
class PasswdWrapperDarwin : public IPasswdWrapperDarwin
{
    public:
        /// @brief Retrieves the passwd entry for the given username.
        /// @param name The username to search for.
        /// @return A pointer to the passwd structure, or nullptr if not found.
        struct passwd* getpwnam(const char* name) override
        {
            return ::getpwnam(name);
        }

        /// @brief Retrieves the passwd entry for the given user ID.
        /// @param uid The user ID to search for.
        /// @return A pointer to the passwd structure, or nullptr if not found.
        struct passwd* getpwuid(uid_t uid) override
        {
            return ::getpwuid(uid);
        }

        /// @brief Rewinds the passwd database to the beginning.
        void setpwent() override
        {
            ::setpwent();
        }

        /// @brief Retrieves the next entry from the passwd database.
        /// @return A pointer to the passwd structure, or nullptr if no more entries.
        struct passwd* getpwent() override
        {
            return ::getpwent();
        }

        /// @brief Closes the passwd database.
        void endpwent() override
        {
            ::endpwent();
        }
};
