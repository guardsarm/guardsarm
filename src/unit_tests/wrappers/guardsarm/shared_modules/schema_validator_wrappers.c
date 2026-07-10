/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#include "schema_validator_wrappers.h"
#include <stddef.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdint.h>

bool __wrap_schema_validator_is_initialized(void) {
    return mock_type(bool);
}

bool __wrap_schema_validator_initialize(void) {
    return mock_type(bool);
}

bool __wrap_schema_validator_validate(const char* indexPattern,
                                      const char* message,
                                      char** errorMessage) {
    check_expected_ptr(indexPattern);
    check_expected_ptr(message);

    if (errorMessage != NULL) {
        *errorMessage = mock_ptr_type(char*);
    }

    return mock_type(bool);
}
