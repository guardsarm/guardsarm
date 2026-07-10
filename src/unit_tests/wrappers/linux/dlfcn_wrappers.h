/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef DLFCN_WRAPPERS_H
#define DLFCN_WRAPPERS_H

#include <dlfcn.h>

void * __wrap_dlopen(const char *filename, int flag);

int __wrap_dlclose(void *handle);

void * __wrap_dlsym(void *handle, const char *symbol);

char * __wrap_dlerror(void);

#endif
