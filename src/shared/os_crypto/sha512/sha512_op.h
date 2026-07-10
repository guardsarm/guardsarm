/*
 * Copyright (C) 2026 GuardSarm, Inc.
 * Mar 14, 2019
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef SHA512_OP_H
#define SHA512_OP_H

#include <sys/types.h>
#include <openssl/sha.h>

#define OS_SHA512_LEN 129

typedef char os_sha512[OS_SHA512_LEN];

int OS_SHA512_File(const char *fname, os_sha512 output, int mode) __attribute((nonnull));
int OS_SHA512_String(const char *str, os_sha512 output);
void OS_SHA512_Hex(const unsigned char md[SHA512_DIGEST_LENGTH], os_sha512 output);

#endif /* SHA512_OP_H */
