/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */


#ifndef SSL_LIB_WRAPPERS_H
#define SSL_LIB_WRAPPERS_H

#include <openssl/ssl.h>

int __wrap_SSL_read(SSL *ssl, void *buf, int num);

int __wrap_SSL_connect(SSL *s);

int __wrap_SSL_get_error(const SSL *s, int i);

int __wrap_SSL_write(SSL *ssl, const void *buf, int num);

SSL *__wrap_SSL_new(SSL_CTX *ctx);

void __wrap_SSL_set_bio(SSL *s, BIO *rbio, BIO *wbio);

#endif
