/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */


#include "wdb_pool_wrappers.h"
#include <stddef.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>

wdb_t * __wrap_wdb_pool_get(const char * name) {
    check_expected(name);
    return mock_ptr_type(wdb_t*);
}

wdb_t * __wrap_wdb_pool_get_or_create(const char * name) {
    check_expected(name);
    return mock_ptr_type(wdb_t*);
}

void __wrap_wdb_pool_leave(__attribute__((unused))wdb_t * node) {
    function_called();
}

char ** __wrap_wdb_pool_keys() {
    return mock_type(char **);
}

void __wrap_wdb_pool_clean() {
    function_called();
}
