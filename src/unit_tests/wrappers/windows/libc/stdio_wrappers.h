/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */


#ifndef STDIO_WRAPPERS_WINDOWS_H
#define STDIO_WRAPPERS_WINDOWS_H

#include <stdio.h>

char * wrap_fgets(char * __s, int __n, FILE * __stream);

int wrap_fprintf(FILE *__stream, const char *__format, ...);

#define fprintf wrap_fprintf
#define fgets wrap_fgets

#endif
