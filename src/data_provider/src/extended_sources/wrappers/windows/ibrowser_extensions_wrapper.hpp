/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#pragma once

#include <string>

class IBrowserExtensionsWrapper
{
    public:
        /// Destructor
        virtual ~IBrowserExtensionsWrapper() = default;
        virtual std::string getHomePath() = 0;
        virtual std::string getApplicationsPath() = 0;
        virtual std::string getUserId(std::string user) = 0;
};
