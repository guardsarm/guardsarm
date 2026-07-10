/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */


#ifndef WMODULES_WRAPPERS_H
#define WMODULES_WRAPPERS_H

#include <stddef.h>
#include "wmodules.h"

int __wrap_wm_sendmsg(int usec,
                      int queue,
                      const char *message,
                      const char *locmsg,
                      char loc);

int __wrap_wm_state_io(const char * tag,
                       int op,
                       void *state,
                       size_t size);

int __wrap_wm_validate_command(const char *command,
                               const char *digest,
                               crypto_type ctype);

void expect_wm_validate_command(const char *command,
                                const char *digest,
                                crypto_type ctype,
                                int ret);

#endif
