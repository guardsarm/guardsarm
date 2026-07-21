/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#include "os_zlib.h"

#include <zlib.h>

unsigned long int os_zlib_compress(const char *src, char *dst,
                                   unsigned long int src_size,
                                   unsigned long int dst_size)
{
    if (compress2((Bytef *)dst,
                  &dst_size,
                  (const Bytef *)src,
                  src_size,
                  Z_BEST_COMPRESSION) == Z_OK) {
        dst[dst_size] = '\0';
        return (dst_size);
    }

    return (0);
}

unsigned long int os_zlib_uncompress(const char *src, char *dst,
                                     unsigned long int src_size,
                                     unsigned long int dst_size)
{
    if (uncompress((Bytef *)dst,
                   &dst_size,
                   (const Bytef *)src,
                   src_size) == Z_OK) {
        dst[dst_size] = '\0';
        return (dst_size);
    }

    return (0);
}
