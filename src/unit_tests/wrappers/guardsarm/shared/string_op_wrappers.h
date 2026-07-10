/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */


#ifndef STRING_OP_WRAPPERS_h
#define STRING_OP_WRAPPERS_h

#include <string.h>

#ifdef WIN32
#include <winsock2.h>
#include <windows.h>

char *__wrap_convert_windows_string(LPCWSTR string);
#endif

int __wrap_wstr_end(char *str, const char *str_end);

char *__wrap_wstr_escape_json(const char * string);

char *__wrap_wstr_replace(const char * string, const char * search, const char * replace);

void __wrap_wstr_split(char *str, char *delim, char *replace_delim, int occurrences, char ***splitted_str);

#endif
