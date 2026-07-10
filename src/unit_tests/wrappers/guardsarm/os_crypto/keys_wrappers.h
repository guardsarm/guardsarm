/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef KEYS_WRAPPERS_H
#define KEYS_WRAPPERS_H


int __wrap_OS_IsAllowedDynamicID(__attribute__((unused)) keystore *keys, const char *id, const char *srcip);

int __wrap_OS_DeleteSocket(__attribute__((unused)) keystore * keys, int sock);

int __wrap_OS_IsAllowedIP(__attribute__((unused)) keystore *keys, const char *srcip);

int __wrap_OS_IsAllowedID(__attribute__((unused)) keystore *keys, const char *id);

keyentry * __wrap_OS_DupKeyEntry(const keyentry * key);

int __wrap_OS_AddSocket(keystore * keys, unsigned int i, int sock);

void __wrap_OS_FreeKey(keyentry *key);

#endif
