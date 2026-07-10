/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#pragma once

#include <string>
#include <pwd.h>
#include "ibrowser_extensions_wrapper.hpp"

#define APP_PATH "/Applications"
#define HOME_PATH "/Users"

class BrowserExtensionsWrapper : public IBrowserExtensionsWrapper
{
    public:
        std::string getApplicationsPath() override
        {
            return std::string(APP_PATH);
        }

        std::string getHomePath() override
        {
            return std::string(HOME_PATH);
        }

        std::string getUserId(std::string user) override
        {
            std::string uid = "";
            struct passwd* pwd = getpwnam(user.c_str());

            if (pwd != nullptr)
            {
                uid = std::to_string(pwd->pw_uid);
            }

            return uid;
        }
};
