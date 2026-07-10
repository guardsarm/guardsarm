/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#include "agent_sync_protocol_wrappers.h"

__attribute__((weak)) void __wrap_asp_sync_module_hook(void) {
}

AgentSyncProtocolHandle* __wrap_asp_create(const char* module, const char* db_path, const MQ_Functions* mq_funcs, asp_logger_t logger, unsigned int sync_end_delay, unsigned int timeout, unsigned int retries, size_t max_eps) {
    check_expected_ptr(module);
    (void)mq_funcs;
    (void)db_path;
    (void)logger;
    check_expected(sync_end_delay);
    check_expected(timeout);
    check_expected(retries);
    check_expected(max_eps);
    return mock_ptr_type(AgentSyncProtocolHandle*);
}

void __wrap_asp_destroy(AgentSyncProtocolHandle* handle) {
    check_expected_ptr(handle);
}

void __wrap_asp_persist_diff(AgentSyncProtocolHandle* handle,
                             const char* id,
                             int operation,
                             const char* index,
                             const char* data,
                             uint64_t version) {
    check_expected_ptr(handle);
    check_expected_ptr(id);
    check_expected(operation);
    check_expected_ptr(index);
    check_expected_ptr(data);
    check_expected(version);
}

bool __wrap_asp_sync_module(AgentSyncProtocolHandle* handle,
                            int mode) {
    check_expected_ptr(handle);
    check_expected(mode);
    __wrap_asp_sync_module_hook();
    return mock_type(bool);
}

void __wrap_asp_persist_diff_in_memory(AgentSyncProtocolHandle* handle,
                                       const char* id,
                                       int operation,
                                       const char* index,
                                       const char* data) {
    check_expected_ptr(handle);
    check_expected_ptr(id);
    check_expected(operation);
    check_expected_ptr(index);
    check_expected_ptr(data);
}

bool __wrap_asp_requires_full_sync(AgentSyncProtocolHandle* handle,
                                   const char* index,
                                   const char* checksum) {
    check_expected_ptr(handle);
    check_expected_ptr(index);
    check_expected_ptr(checksum);
    return mock_type(bool);
}

bool __wrap_asp_parse_response_buffer(AgentSyncProtocolHandle* handle, const uint8_t* data, size_t length) {
    check_expected_ptr(handle);
    check_expected_ptr(data);
    check_expected(length);
    return mock_type(bool);
}

void __wrap_asp_clear_in_memory_data(AgentSyncProtocolHandle* handle) {
    check_expected_ptr(handle);
}

bool __wrap_asp_sync_metadata_or_groups(AgentSyncProtocolHandle* handle,
                                        int mode) {
    check_expected_ptr(handle);
    check_expected(mode);
    return mock_type(bool);
}

bool __wrap_asp_notify_data_clean(AgentSyncProtocolHandle* handle,
                                  const char** indices,
                                  size_t indices_count) {
    check_expected_ptr(handle);
    check_expected_ptr(indices);
    check_expected(indices_count);
    return mock_type(bool);
}

void __wrap_asp_delete_database(AgentSyncProtocolHandle* handle) {
    check_expected_ptr(handle);
}
