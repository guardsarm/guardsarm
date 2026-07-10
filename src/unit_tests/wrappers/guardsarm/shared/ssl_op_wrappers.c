/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#include "ssl_op_wrappers.h"
#include <stddef.h>
#include <stdarg.h>
#include <setjmp.h>
#include <stdint.h>
#include <cmocka.h>

SSL_CTX *__wrap_os_ssl_keys(int is_server, const char *os_dir, const char *ciphers, const char *cert, const char *key,
                            const char *ca_cert, int auto_method) {
    check_expected(is_server);
    check_expected(os_dir);
    check_expected(ciphers);
    check_expected(cert);
    check_expected(key);
    check_expected(ca_cert);
    check_expected(auto_method);
    return mock_ptr_type(SSL_CTX *);
}
