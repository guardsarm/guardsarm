/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef MSGS_WRAPPERS_H
#define MSGS_WRAPPERS_H

#include <sys/types.h>
#include "sec.h"

size_t __wrap_CreateSecMSG(__attribute__((unused)) keystore *keys, const char *msg, size_t msg_length, char *msg_encrypted, unsigned int id);
int __wrap_ReadSecMSG(keystore *keys, char *buffer, __attribute__((unused)) char *cleartext, int id, __attribute__((unused)) unsigned int buffer_size, size_t *final_size, const char *srcip, char **output);

#endif
