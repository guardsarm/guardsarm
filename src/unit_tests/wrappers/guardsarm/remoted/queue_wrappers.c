/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */


#include <stddef.h>
#include <stdarg.h>
#include <setjmp.h>
#include <stdint.h>
#include <cmocka.h>
#include <stdio.h>
#include <stdlib.h>
#include <shared.h>
#include "os_net.h"
#include "queue_wrappers.h"

size_t __wrap_rem_get_qsize() {
    return mock();
}

size_t __wrap_rem_get_tsize() {
    return mock();
}

int __wrap_rem_msgpush(__attribute__((unused)) const char * buffer, unsigned long size, struct sockaddr_storage * addr, int sock) {
    check_expected(sock);
    check_expected_ptr(addr);
    check_expected(size);

    return mock();
}
