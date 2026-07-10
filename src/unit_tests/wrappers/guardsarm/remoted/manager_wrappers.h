/*
 * GuardSarm Shared Configuration Manager
 * Copyright (C) 2026 GuardSarm, Inc.
 * Feb 1, 2023.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef REM_MANAGER_WRAPPERS_H
#define REM_MANAGER_WRAPPERS_H

#include <cJSON.h>
#include "shared.h"

cJSON *__wrap_assign_group_to_agent(const char *agent_id, const char *md5);

void __wrap_save_controlmsg(const keyentry * key, char *r_msg, __attribute__((unused)) size_t msg_length, int *wdb_sock);

int __wrap_validate_control_msg(const keyentry * key, char *r_msg, size_t msg_length, char **cleaned_msg, int *is_startup, int *is_shutdown);


#endif /* REM_MANAGER_WRAPPERS_H */
