/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */


#include "x509_wrapppers.h"
#include <stddef.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>

int __wrap_X509_sign(X509 *x, EVP_PKEY *pkey, const EVP_MD *md) {
    return mock_type(int);
}

X509 *__wrap_X509_new(void) {
    if (mock()) {
        return __real_X509_new();
    }
    return mock_type(X509 *);
}
