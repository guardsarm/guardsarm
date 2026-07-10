/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */


#ifndef DIGEST_WRAPPERS_H
#define DIGEST_WRAPPERS_H

#ifdef WIN32
#include <winsock2.h>
#include <windows.h>
#endif
#include <stddef.h>
#include <openssl/evp.h>

int __wrap_EVP_DigestUpdate(EVP_MD_CTX *ctx,
                            const void *data,
                            size_t count);

#endif
