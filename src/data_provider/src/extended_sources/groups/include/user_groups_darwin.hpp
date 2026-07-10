/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#pragma once

#include <chrono>
#include <set>
#include <unordered_map>

#include "json.hpp"
#include "igroup_wrapper.hpp"
#include "ipasswd_wrapper.hpp"
#include "iopen_directory_utils_wrapper.hpp"

#define EXPECTED_GROUPS_MAX 64

class UserGroupsProvider
{
    public:
        explicit UserGroupsProvider(std::shared_ptr<IGroupWrapperDarwin> groupWrapper,
                                    std::shared_ptr<IPasswdWrapperDarwin> passwdWrapper,
                                    std::shared_ptr<IODUtilsWrapper> odWrapper);
        UserGroupsProvider();
        nlohmann::json collect(const std::set<uid_t>& uids = {});

        nlohmann::json getUserNamesByGid(const std::set<gid_t>& gids);

        nlohmann::json getGroupNamesByUid(const std::set<uid_t>& uids);

        static void resetCache();

    private:
        static bool validateCache();

        std::shared_ptr<IGroupWrapperDarwin> m_groupWrapper;
        std::shared_ptr<IPasswdWrapperDarwin> m_passwdWrapper;
        std::shared_ptr<IODUtilsWrapper> m_odWrapper;

        struct UserInfo
        {
            const char* name;
            uid_t uid;
            gid_t gid;
        };

        void getGroupsForUser(nlohmann::json& results, const UserInfo& user);
        void addGroupsToResults(nlohmann::json& results, uid_t uid, const gid_t* groups, int ngroups);
        std::vector<gid_t> getGroupIdsForUser(const UserInfo& user);
        std::vector<gid_t> getUserGroups(uid_t uid, const char* username, gid_t gid);

};
