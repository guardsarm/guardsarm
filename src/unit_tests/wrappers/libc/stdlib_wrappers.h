/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */


#ifndef STDLIB_WRAPPERS_H
#define STDLIB_WRAPPERS_H

#include <stddef.h>

int __wrap_atexit(void (*callback)(void));

char *__wrap_realpath(const char *path, char *resolved_path);
char *__wrap___realpath_chk(const char *path, char *resolved_path, size_t resolved_len);

int __wrap_system(const char *__command);
void expect_system(int ret);

int __wrap_mkstemp(char *template);

#endif
