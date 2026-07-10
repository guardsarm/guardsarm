/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef FIM_RECOVERY_WRAPPERS_H
#define FIM_RECOVERY_WRAPPERS_H

#include <stdbool.h>
#include <stdint.h>

// Forward declaration for AgentSyncProtocolHandle
typedef struct AgentSyncProtocolHandle AgentSyncProtocolHandle;

// Forward declaration for OSList
typedef struct _OSList OSList;

/**
 * @brief Wrapper for fim_recovery_persist_table_and_resync
 */
void __wrap_fim_recovery_persist_table_and_resync(char* table_name,
                                                   AgentSyncProtocolHandle* handle,
                                                   const OSList* directories_list);

/**
 * @brief Wrapper for fim_recovery_check_if_full_sync_required
 */
bool __wrap_fim_recovery_check_if_full_sync_required(char* table_name,
                                                      AgentSyncProtocolHandle* handle);

/**
 * @brief Wrapper for fim_recovery_integrity_interval_has_elapsed
 */
bool __wrap_fim_recovery_integrity_interval_has_elapsed(char* table_name, int64_t integrity_interval);

#endif
