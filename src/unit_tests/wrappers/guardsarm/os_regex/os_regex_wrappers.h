/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef OS_REGEX_WRAPPERS_H
#define OS_REGEX_WRAPPERS_H

#include "os_regex.h"

int __wrap_OSRegex_Compile(const char* pattern, OSRegex* reg, int flags);

const char* __wrap_OSRegex_Execute(const char* str, OSRegex* reg);

int __wrap_OS_StrIsNum(const char* str);

int __wrap_OSMatch_Execute(const char* str, size_t str_len, OSMatch* reg);

const char* __wrap_OSRegex_Execute_ex(const char* str, OSRegex* reg, regex_matching* regex_match);

void __wrap_OSRegex_FreePattern(OSRegex* reg);

#endif
