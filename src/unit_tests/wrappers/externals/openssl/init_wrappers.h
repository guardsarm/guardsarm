/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */


#ifndef INIT_WRAPPERS_H
#define INIT_WRAPPERS_H

#include <stdint.h>
#include <openssl/crypto.h>

int __wrap_OPENSSL_init_crypto(uint64_t opts,
                               const OPENSSL_INIT_SETTINGS *settings);

#endif
