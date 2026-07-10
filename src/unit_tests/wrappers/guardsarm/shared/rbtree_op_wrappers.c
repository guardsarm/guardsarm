/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#include "rbtree_op_wrappers.h"
#include <stddef.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>

void * __wrap_rbtree_insert(__attribute__((unused)) rb_tree * tree,
                            __attribute__((unused)) const char * key,
                            __attribute__((unused)) void * value) {
    return NULL;
}

char **__wrap_rbtree_keys(__attribute__((unused)) const rb_tree *tree) {
    return mock_type(char **);
}

void *__wrap_rbtree_get(const rb_tree *tree, const char * key) {
    check_expected(tree);
    check_expected(key);
    return mock_type(void*);
}
