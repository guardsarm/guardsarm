/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#include <stddef.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>

void __wrap_audit_read_events(int *audit_sock, int mode) {
    check_expected(mode);
    *audit_sock = mock_type(int);
}

int __wrap_init_auditd_socket(void) {
    return mock_type(int);
}
