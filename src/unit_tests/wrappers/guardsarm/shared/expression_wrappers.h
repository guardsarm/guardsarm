/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef EXPRESSION_WRAPPERS_H
#define EXPRESSION_WRAPPERS_H

#include "expression.h"
#include "shared.h"
#include <stdbool.h>

bool __wrap_w_expression_match(__attribute__((unused)) w_expression_t* expression,
                               __attribute__((unused)) const char* str_test,
                               __attribute__((unused)) const char** end_match,
                               regex_matching* regex_match);

void __wrap_w_calloc_expression_t(__attribute__((unused)) w_expression_t** var, w_exp_type_t type);

void __wrap_w_free_expression_t(__attribute__((unused)) w_expression_t** var);

bool __wrap_w_expression_compile(__attribute__((unused)) w_expression_t* expression,
                                 __attribute__((unused)) char* pattern,
                                 __attribute__((unused)) int flags);

#endif
