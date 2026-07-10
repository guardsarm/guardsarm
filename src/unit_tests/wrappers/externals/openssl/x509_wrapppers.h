/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */


#include <openssl/x509.h>

int __wrap_X509_sign(X509 *x, EVP_PKEY *pkey, const EVP_MD *md);

X509 *__wrap_X509_new(void);
extern X509 *__real_X509_new(void);
