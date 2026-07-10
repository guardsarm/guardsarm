/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */


#include "evp_wrappers.h"
#include <stddef.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>


EVP_PKEY *__wrap_EVP_PKEY_new(void) {
    if (mock()) {
        return __real_EVP_PKEY_new();
    }

    return mock_type(EVP_PKEY *);
}
