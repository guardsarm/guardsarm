/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */


#ifndef MD5_OP_WRAPPERS_H
#define MD5_OP_WRAPPERS_H

#include <string.h>
#include <sys/types.h>

typedef char os_md5[33];
typedef char os_sha1[41];
typedef char os_sha256[65];

int __wrap_OS_MD5_File(const char *fname, os_md5 output, int mode);
int __wrap_OS_MD5_Str(const char *str, ssize_t length, os_md5 output);
void expect_OS_MD5_File_call(const char *fname, os_md5 output, int mode, int ret);

int __wrap_OS_MD5_SHA1_SHA256_File(const char *fname, os_md5 md5output, os_sha1 sha1output,
                                   os_sha256 sha256output, int mode, size_t max_size);

/**
 * @brief This function loads the expect and will return of the function OS_MD5_SHA1_SHA256_File
 */
void expect_OS_MD5_SHA1_SHA256_File_call(char *file,
                                         char *md5,
                                         char *sha1,
                                         char *sha256,
                                         int mode,
                                         int max_size,
                                         int ret);
#endif
