/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */


#ifndef SYSCOM_WRAPPERS_H
#define SYSCOM_WRAPPERS_H

#include <stddef.h>

size_t __wrap_syscom_dispatch(char * command, size_t length, char ** output);

#endif
