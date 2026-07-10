/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#include "bio_wrappers.h"
#include <stddef.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>

BIO *__wrap_BIO_new_socket(__attribute__((unused)) int sock,
                           __attribute__((unused)) int close_flag) {
    return NULL;
}
