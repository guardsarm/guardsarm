/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#include "stat64_wrappers.h"
#include <stddef.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>
#include <string.h>

int wrap__stat64(const char * __file, struct _stat64 * __buf) {
    struct _stat64 * mock_buf;

    check_expected(__file);
    mock_buf = mock_type(struct _stat64 *);
    if (mock_buf != NULL) {
        memcpy(__buf, mock_buf, sizeof(struct _stat64));
    }
    return mock_type(int);
}
