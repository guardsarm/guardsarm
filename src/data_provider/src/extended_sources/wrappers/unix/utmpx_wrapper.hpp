/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#include "iutmpx_wrapper.hpp"
#include <utmpx.h>

/// @brief Wrapper class for utmpx-related system calls.
///
/// Provides access to user session records using the utmpx API.
class UtmpxWrapper : public IUtmpxWrapper
{
    public:
        /// @brief Sets the utmpx file to be used by the library functions.
        /// @param file Path to the utmpx file.
        void utmpxname(const char* file) override
        {
            ::utmpxname(file);
        }

        /// @brief Resets the input stream to the beginning of the utmpx file.
        void setutxent() override
        {
            ::setutxent();
        }

        /// @brief Closes the utmpx file.
        void endutxent() override
        {
            ::endutxent();
        }

        /// @brief Reads the next entry from the utmpx file.
        /// @return A pointer to the next utmpx structure, or nullptr if none.
        struct utmpx* getutxent() override
        {
            return ::getutxent();
        }
};
