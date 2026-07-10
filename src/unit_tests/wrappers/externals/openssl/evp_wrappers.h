/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */


#include <openssl/evp.h>

EVP_PKEY * __wrap_EVP_PKEY_new(void);
extern EVP_PKEY * __real_EVP_PKEY_new(void);
