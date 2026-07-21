/* Copyright (C) 2026 GuardSarm, Inc.
 * All right reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

/* OS_crypto/md5 Library
 * APIs for many crypto operations
 */

#ifndef MD5_OP_H
#define MD5_OP_H

#include <sys/types.h>

typedef char os_md5[33];

int OS_MD5_File(const char *fname, os_md5 output, int mode) __attribute((nonnull));
int OS_MD5_Str(const char *str, ssize_t length, os_md5 output) __attribute((nonnull));

#endif /* MD5_OP_H */
