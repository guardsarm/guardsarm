/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#pragma once

#include <functional>
#include <string>

#include "agent_sync_protocol_c_interface_types.h"

using LoggerFunc = std::function<void(modules_log_level_t, const std::string&)>;

/// @brief Result status for sync operations, especially for integrity checks
enum class SyncResult
{
    SUCCESS,             ///< Operation completed successfully
    COMMUNICATION_ERROR, ///< Manager is offline or unreachable
    CHECKSUM_ERROR,      ///< Checksum validation failed
    GENERIC_ERROR        ///< Generic synchronization error
};
