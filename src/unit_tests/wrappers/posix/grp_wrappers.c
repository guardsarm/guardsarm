/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#include "grp_wrappers.h"
#include <stddef.h>
#include <stdarg.h>
#include <setjmp.h>
#include <stdint.h>
#include <cmocka.h>
#include <string.h>

#ifndef WIN32
struct group *__wrap_getgrgid(__attribute__((unused)) gid_t gid) {
    return mock_ptr_type(struct group*);
}

int __wrap_getgrnam_r(const char *name, struct group *grp,__attribute__((unused)) char *buf, size_t buflen, struct group **result) {
    *result = NULL;

    if (buflen < 1024) {
        return ERANGE;
    }

    if (strcmp(name, "guardsarm") == 0) {
        grp->gr_gid = 1000;
        *result = grp;
    }

    return 0;
}
#endif
