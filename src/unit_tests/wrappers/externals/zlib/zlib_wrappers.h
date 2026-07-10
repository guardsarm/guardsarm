/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */


#ifndef ZLIB_WRAPPERS_H
#define ZLIB_WRAPPERS_H

#include "../../../../external/zlib/zlib.h"

int __wrap_gzread(gzFile gz_fd,
                  void* buf,
                  int len);

gzFile __wrap_gzopen(const char * file,
                     const char * mode);

int __wrap_gzclose(gzFile file);

int __wrap_gzeof(gzFile file);

const char * __wrap_gzerror(gzFile file,
                            __attribute__((unused)) int *errnum);

int __wrap_gzwrite(gzFile file,
                   voidpc buf,
                   unsigned int len);

#endif
