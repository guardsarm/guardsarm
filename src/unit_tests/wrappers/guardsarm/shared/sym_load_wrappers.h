/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef SYM_LOAD_WRAPPERS_H
#define SYM_LOAD_WRAPPERS_H
#include "sym_load.h"

void* __wrap_so_get_module_handle_on_path(const char *path, const char *so);

void* __wrap_so_get_module_handle(const char *so);

void* __wrap_so_get_function_sym(void *handle, const char *function_name);

int __wrap_so_free_library(void *handle);

#endif
