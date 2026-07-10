/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */


#ifndef CHECK_CERT_OP_WRAPPERS_H
#define CHECK_CERT_OP_WRAPPERS_H

#include <openssl/ssl.h>

int __wrap_check_x509_cert(const SSL *ssl, const char *manager);

#endif
