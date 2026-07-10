/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#include "pwd_wrappers.h"

#ifndef WIN32
#include <stddef.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>
#include <errno.h>
#include <string.h>


int __wrap_getpwnam_r(const char *name,
                      struct passwd *pwd,
                      __attribute__((unused))  char *buf,
                      size_t buflen,
                      struct passwd **result) {
    *result = NULL;

    if (buflen < 1024) {
        return ERANGE;
    }

    if (strcmp(name, "guardsarm") == 0) {
        pwd->pw_uid = 1000;
        *result = pwd;
    }

    return 0;
}

int __wrap_getpwuid_r(__attribute__((unused)) uid_t uid,
                      struct passwd *pwd,
                      __attribute__((unused)) char *buf,
                      __attribute__((unused)) size_t buflen,
                      struct passwd **result) {
    pwd->pw_name = mock_type(char*);
    *result = mock_type(struct passwd*);
    return mock();
}
#endif
