/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#include "privsep_op_wrappers.h"
#include <stddef.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>

#include <string.h>

#ifndef WIN32
struct group *__wrap_w_getgrgid(gid_t gid, struct group *grp, char *buf, int buflen) {
    struct group *mock_grp;
    char *mock_buf;
    check_expected(gid);

    mock_grp = mock_type(struct group *);

    if (mock_grp && grp) {
        memcpy(grp, mock_grp, sizeof(char **) + 2 * sizeof(char *) + sizeof(gid_t));
    }

    mock_buf = mock_type(char *);
    if (mock_buf && buf) {
        strncpy(buf, mock_buf, buflen);
    }

    if (mock()) {
        return grp;
    } else {
        return NULL;
    }
}
#endif

int __wrap_Privsep_GetUser(const char *name) {
    check_expected(name);

    return mock();
}

int __wrap_Privsep_GetGroup(const char *name) {
    check_expected(name);

    return mock();
}
