/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */


#ifndef SYSCHECK_AUDIT_WRAPPERS
#define SYSCHECK_AUDIT_WRAPPERS

void __wrap_audit_read_events(int *audit_sock, int mode);

int __wrap_init_auditd_socket(void);
#endif // SYSCHECK_AUDIT_WRAPPERS
