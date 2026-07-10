/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */


#ifndef STRING_WRAPPERS_H
#define STRING_WRAPPERS_H

#include <string.h>

char *__wrap_strerror (int __errnum);

size_t __wrap_strlen(const char *s);
#endif
