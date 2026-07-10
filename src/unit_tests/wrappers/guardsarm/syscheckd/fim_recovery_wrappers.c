/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#include "fim_recovery_wrappers.h"
#include <stddef.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>

void __wrap_fim_recovery_persist_table_and_resync(__attribute__((unused)) char* table_name,
                                                   __attribute__((unused)) AgentSyncProtocolHandle* handle,
                                                   __attribute__((unused)) const OSList* directories_list) {
    function_called();
}

bool __wrap_fim_recovery_check_if_full_sync_required(__attribute__((unused)) char* table_name,
                                                      __attribute__((unused)) AgentSyncProtocolHandle* handle) {
    function_called();
    return mock_type(bool);
}

bool __wrap_fim_recovery_integrity_interval_has_elapsed(__attribute__((unused)) char* table_name,
                                                         __attribute__((unused)) int64_t integrity_interval) {
    function_called();
    return mock_type(bool);
}
