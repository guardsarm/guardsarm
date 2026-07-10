/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */


#ifndef WIN_WHODATA_WRAPPERS_H
#define WIN_WHODATA_WRAPPERS_H

#include "syscheck-config.h"

int __wrap_run_whodata_scan();

int __wrap_set_winsacl(const char *dir, directory_t *configuration);

int __wrap_whodata_audit_start();

#endif
