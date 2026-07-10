/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#pragma once

#include <uuid/uuid.h>

// Interface for the uuid wrapper
class IUUIDWrapper
{
    public:
        /// Destructor
        virtual ~IUUIDWrapper() = default;

        /// @brief Converts a UID to a UUID.
        /// @param uid The user ID to convert.
        /// @param uuid Output parameter that will contain the corresponding UUID.
        virtual void uidToUUID(uid_t uid, uuid_t& uuid) = 0;

        /// @brief Converts a UUID to its string representation.
        /// @param uuid The UUID to convert.
        /// @param str Output buffer that will contain the string representation.
        virtual void uuidToString(const uuid_t& uuid, uuid_string_t& str) = 0;
};
