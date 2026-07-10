/*
 * Copyright (C) 2026 GuardSarm, Inc.
 * April, 2020.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef BZIP2_OP_H
#define BZIP2_OP_H

#include <bzlib.h>

#define BZIP2_BUFFER_SIZE 4096

/**
 * @brief bzpi2 library, function to compress
 *
 * @param file File path to compress
 * @param filebz2 File name compressed
 *
 * @retval 0 on success
 * @retval -1 on error
 */
int bzip2_compress(const char *file, const char *filebz2);

/**
 * @brief bzpi2 library, function to uncompress
 *
 * @param filebz2 File path to uncompress
 * @param file File name uncompressed
 *
 * @retval 0 on success
 * @retval -1 on error
 */
int bzip2_uncompress(const char *filebz2, const char *file);

#endif /* BZIP2_OP_H */
