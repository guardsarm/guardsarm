/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */


#ifndef AUDIT_RULE_HANDLING_WRAPPERS
#define AUDIT_RULE_HANDLING_WRAPPERS

void __wrap_fim_rules_initial_load();

void __wrap_add_whodata_directory(const char *path);

void __wrap_remove_audit_rule_syscheck(const char *path);

void __wrap_fim_audit_reload_rules();

int __wrap_fim_manipulated_audit_rules();

#endif // AUDIT_RULE_HANDLING_WRAPPERS
