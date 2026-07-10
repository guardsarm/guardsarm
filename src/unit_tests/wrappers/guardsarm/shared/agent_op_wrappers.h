/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */


#ifndef AGENT_OP_WRAPPERS_H
#define AGENT_OP_WRAPPERS_H

#include "stddef.h"
#include "stdbool.h"
#include "cJSON.h"

int __wrap_auth_connect();
char* __wrap_get_agent_id_from_name(__attribute__((unused)) char *agent_name);
char* __wrap_getPrimaryIP(void);

cJSON* __wrap_w_create_sendsync_payload(const char *daemon_name, cJSON *message);
int __wrap_w_send_clustered_message(const char* command, const char* payload, char* response);
bool __wrap_w_query_agentd(const char *module, const char *query, char *output, size_t output_size);
bool __wrap_fetch_document_limits_from_agentd();

#endif
