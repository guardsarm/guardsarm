/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */


#ifndef AUDIT_OP_WRAPPERS_H
#define AUDIT_OP_WRAPPERS_H

int __wrap_audit_add_rule(const char *path, int perms, const char *key);

int __wrap_audit_get_rule_list(int fd);

int __wrap_audit_restart();

int __wrap_audit_set_db_consistency();

int __wrap_search_audit_rule(const char *path, int perms, const char *key);

int __wrap_audit_delete_rule(const char *path, int perms, const char *key);

#endif
