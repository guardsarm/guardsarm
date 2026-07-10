/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#include "digest_wrappers.h"
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>
#include "../../common.h"
#include <openssl/evp.h>

extern int __real_EVP_DigestUpdate(EVP_MD_CTX *ctx,const void *data, size_t count);
int __wrap_EVP_DigestUpdate(EVP_MD_CTX *ctx,
                            const void *data,
                            size_t count) {
   if (test_mode) {
      check_expected(data);
      check_expected(count);
      return mock();
   }
   else {
     return __real_EVP_DigestUpdate(ctx, data, count);
   }
}
