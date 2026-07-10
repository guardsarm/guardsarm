/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#pragma once

#include <winsock2.h>
#include <windows.h>
#include <cstdint>
#include <limits>
#include <string>
#include <vector>

/// @brief Represents a Windows group with its associated properties.
struct Group
{
    std::uint32_t generation{0};
    std::uint32_t gid{std::numeric_limits<std::uint32_t>::max()};
    std::string sid;
    std::string groupname;
    std::string comment;

    bool operator==(const Group& other) const
    {
        return gid == other.gid && sid == other.sid &&
               groupname == other.groupname && comment == other.comment;
    }
};

// Interface for the windows groups helper wrapper
class IGroupsHelper
{
    public:
        /// Destructor
        virtual ~IGroupsHelper() = default;

        /// @brief Processes local Windows groups and collects group data.
        /// @return Vector of Group objects representing local groups.
        virtual std::vector<Group> processLocalGroups() = 0;
};
