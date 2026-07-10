/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */


#ifndef BIO_WRAPPERS_H
#define BIO_WRAPPERS_H

#include <openssl/ssl.h>

BIO *__wrap_BIO_new_socket(int sock, int close_flag);

#endif
