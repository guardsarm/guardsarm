/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef REMOTED_OP_WRAPPERS_H
#define REMOTED_OP_WRAPPERS_H

#include "wdb.h"

int __wrap_parse_agent_update_msg(char* msg, __attribute__((unused)) agent_info_data* agent_data);

int __wrap_parse_json_keepalive(const char* json_str,
                                __attribute__((unused)) agent_info_data* agent_data,
                                __attribute__((unused)) char*** groups_out,
                                __attribute__((unused)) size_t* groups_count_out,
                                __attribute__((unused)) char** cluster_name_out,
                                __attribute__((unused)) char** cluster_node_out);

#endif
