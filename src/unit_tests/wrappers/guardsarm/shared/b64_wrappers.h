/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */


#ifndef B64_WRAPPERS_H
#define B64_WRAPPERS_H

#include <sys/types.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef WIN32
#include <stdint.h>
#include <winsock2.h>
#include <windows.h>
#endif

char *__wrap_encode_base64(int size, const char *src);
void expect_encode_base64(int size, const char *src, char * ret);

char *__wrap_decode_base64(const char *src);
void expect_decode_base64(const char *src, char * ret);

#endif
