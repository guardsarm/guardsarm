/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef SCHEMA_VALIDATOR_WRAPPERS_H
#define SCHEMA_VALIDATOR_WRAPPERS_H

#include <stdbool.h>

/**
 * @brief Wrapper for schema_validator_is_initialized
 */
bool __wrap_schema_validator_is_initialized(void);

/**
 * @brief Wrapper for schema_validator_initialize
 */
bool __wrap_schema_validator_initialize(void);

/**
 * @brief Wrapper for schema_validator_validate
 */
bool __wrap_schema_validator_validate(const char* indexPattern,
                                      const char* message,
                                      char** errorMessage);

#endif
