/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#include "audit_rule_handling_wrappers.h"
#include <stddef.h>
#include <stdarg.h>
#include <setjmp.h>
#include <stdint.h>
#include <cmocka.h>


void __wrap_fim_rules_initial_load() {
    function_called();
}

void __wrap_add_whodata_directory(const char *path) {
    check_expected_ptr(path);
}

void __wrap_remove_audit_rule_syscheck(const char *path) {
    check_expected_ptr(path);
}

void __wrap_fim_audit_reload_rules() {
    function_called();
}

int __wrap_fim_manipulated_audit_rules() {
    return mock_type(int);
}
