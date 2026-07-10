/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef WIN32

#include "select_wrappers.h"
#include <stddef.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>


int __wrap_select(__attribute__((unused)) int nfds,
                  __attribute__((unused)) fd_set *restrict readfds,
                  __attribute__((unused)) fd_set *restrict writefds,
                  __attribute__((unused)) fd_set *restrict errorfds,
                  __attribute__((unused)) struct timeval *restrict timeout) {
    return mock();
}

#endif
