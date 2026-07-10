/*
 * Copyright (C) 2026 GuardSarm, Inc.
 * Jun 21, 2017.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef HMAC_H
#define HMAC_H

#define HMAC_SHA1_BLOCKSIZE 64

int OS_HMAC_SHA1_Str(const char *key, const char *text, os_sha1 output) __attribute((nonnull));
int OS_HMAC_SHA1_File(const char *key, const char *file_path, os_sha1 output, int mode) __attribute((nonnull));

#endif /* HMAC_H */
