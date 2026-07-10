/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#pragma once

#include <uuid/uuid.h>
#include <membership.h>

#include "iuuid_wrapper.hpp"

/// @brief Wrapper class for UUID-related operations.
///
/// Provides methods to convert between user IDs (UIDs) and UUIDs,
/// and to convert UUIDs to their string representations.
class UUIDWrapper : public IUUIDWrapper
{
    public:
        /// @brief Converts a UID to a UUID.
        /// @param uid The user ID to convert.
        /// @param uuid Output parameter that will contain the corresponding UUID.
        void uidToUUID(uid_t uid, uuid_t& uuid) override
        {
            mbr_uid_to_uuid(uid, uuid);
        }

        /// @brief Converts a UUID to its string representation.
        /// @param uuid The UUID to convert.
        /// @param str Output buffer that will contain the string representation.
        void uuidToString(const uuid_t& uuid, uuid_string_t& str) override
        {
            uuid_unparse(uuid, str);
        }
};
