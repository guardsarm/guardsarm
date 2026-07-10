/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#pragma once

#include "ishadow_wrapper.hpp"

/// ShadowWrapper class
/// This class is responsible for providing an interface to the shadow file functions.
class ShadowWrapper : public IShadowWrapper
{
    public:
        /// @brief lckpwdf locks the shadow file
        /// @return Returns 0 on success, -1 on error
        int lckpwdf()
        {
            return ::lckpwdf();
        }

        /// @brief setspent sets the position of the shadow file to the beginning
        void setspent()
        {
            return ::setspent();
        }

        /// @brief getspent reads the next entry from the shadow file
        /// @return Returns a pointer to the next entry in the shadow file
        struct spwd* getspent()
        {
            return ::getspent();
        }

        /// @brief endspent closes the shadow file
        void endspent()
        {
            return ::endspent();
        }

        /// @brief ulckpwdf unlocks the shadow file
        /// @return Returns 0 on success, -1 on error
        int ulckpwdf()
        {
            return ::ulckpwdf();
        }

};
