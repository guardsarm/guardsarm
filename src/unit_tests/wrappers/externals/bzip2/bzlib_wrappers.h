/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */


#ifndef BZLIB_WRAPPERS_H
#define BZLIB_WRAPPERS_H

#include "bzlib.h"

int __wrap_BZ2_bzRead(int* bzerror,
                      BZFILE* f,
                      void* buf,
                      int len);

void __wrap_BZ2_bzReadClose(int* bzerror,
                            BZFILE* f);

BZFILE* __wrap_BZ2_bzReadOpen(int* bzerror,
                          FILE* f,
                          int small,
                          int verbosity,
                          void* unused,
                          int nUnused);

void __wrap_BZ2_bzWrite(int* bzerror,
                       BZFILE* f,
                       void* buf,
                       int len);

void __wrap_BZ2_bzWriteClose(int* bzerror,
                               BZFILE* f,
                               int abandon,
                               unsigned int* nbytes_in,
                               unsigned int* nbytes_out);

BZFILE* __wrap_BZ2_bzWriteOpen(int* bzerror,
                               FILE* f,
                               int blockSize100k,
                               int verbosity,
                               int workFactor);

#ifndef TEST_WINAGENT
int __wrap_bzip2_uncompress(const char *filebz2,
                            const char *file);
#endif

#endif
