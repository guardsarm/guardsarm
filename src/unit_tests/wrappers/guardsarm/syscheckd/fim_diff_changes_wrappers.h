/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */


#ifndef DIFF_CHANGES_WRAPPERS_H
#define DIFF_CHANGES_WRAPPERS_H

#ifdef WIN32
#include <winsock2.h>
#include <windows.h>
#include "syscheck-config.h"
#endif


char *__wrap_fim_file_diff(const char *filename);

int __wrap_fim_diff_process_delete_file(const char *file_name);

/**
 * @brief This function loads the expect and will return of the function fim_file_diff
 */
void expect_fim_file_diff(const char *filename, char *ret);

/**
 * @brief This function loads the expect and will return of the function fim_diff_process_delete_file
 */
void expect_fim_diff_process_delete_file(const char *filename, int ret);

#ifdef WIN32
char *__wrap_fim_registry_value_diff(const char *key_name,
                                     const char *value_name,
                                     const char *value_data,
                                     DWORD data_type,
                                     const registry_t *configuration);

/**
 * @brief This function loads the expect and will return of the function fim_registry_value_diff
 */
void expect_fim_registry_value_diff(const char *key_name,
                                    const char *value_name,
                                    const char *value_data,
                                    DWORD data_size,
                                    DWORD data_type,
                                    char *ret);
#endif
#endif /* DIFF_CHANGES_WRAPPERS_H */
