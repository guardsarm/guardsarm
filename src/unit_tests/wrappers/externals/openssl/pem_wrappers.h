/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */


#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <stdio.h>

int __wrap_PEM_write_PrivateKey(FILE *out,
                                const EVP_PKEY *x,
                                const EVP_CIPHER *enc,
                                const unsigned char *kstr,
                                int klen,
                                pem_password_cb *cb,
                                void *u);

int __wrap_PEM_write_X509(FILE *out, const X509 *x);
