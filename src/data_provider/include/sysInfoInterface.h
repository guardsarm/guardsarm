/*
 * GuardSarm SysInfo
 * Copyright (C) 2026 GuardSarm, Inc.
 * November 9, 2020.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef _SYS_INFO_INTERFACE
#define _SYS_INFO_INTERFACE

#include "json.hpp"

class ISysInfo
{
    public:
        ISysInfo() = default;
        // LCOV_EXCL_START
        virtual ~ISysInfo() = default;
        // LCOV_EXCL_STOP
        virtual nlohmann::json hardware() = 0;
        virtual nlohmann::json packages() = 0;
        virtual nlohmann::json os() = 0;
        virtual nlohmann::json processes() = 0;
        virtual nlohmann::json networks() = 0;
        virtual nlohmann::json ports() = 0;
        virtual nlohmann::json hotfixes() = 0;
        virtual nlohmann::json groups() = 0;
        virtual nlohmann::json users() = 0;
        virtual nlohmann::json services() = 0;
        virtual nlohmann::json browserExtensions() = 0;
        virtual void packages(std::function<void(nlohmann::json&)>) = 0;
        virtual void processes(std::function<void(nlohmann::json&)>) = 0;

};

#endif //_SYS_INFO_INTERFACE
