/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */


#ifndef DEBUG_OP_WRAPPERS_H
#define DEBUG_OP_WRAPPERS_H

#include "defs.h"

int __wrap_isChroot();

void __wrap__mdebug1(const char * file,
                     int line,
                     const char * func,
                     const char *msg, ...);

void __wrap__mdebug2(const char * file,
                     int line,
                     const char * func,
                     const char *msg, ...);

void __wrap__merror(const char * file,
                    int line,
                    const char * func,
                    const char *msg, ...);

void __wrap__merror_exit(const char * file,
                         int line,
                         const char * func,
                         const char *msg, ...);

void __wrap__mferror(const char * file,
                    int line,
                    const char * func,
                    const char *msg, ...);

void __wrap__minfo(const char * file,
                  int line,
                  const char * func,
                  const char *msg, ...);

void __wrap__mtdebug1(const char *tag,
                      const char * file,
                      int line,
                      const char * func,
                      const char *msg, ...);

void __wrap__mtdebug2(const char *tag,
                      const char * file,
                      int line,
                      const char * func,
                      const char *msg, ...);

void __wrap__mterror(const char *tag,
                      const char * file,
                      int line,
                      const char * func,
                      const char *msg, ...);

void __wrap__mterror_exit(const char *tag,
                          const char * file,
                          int line,
                          const char * func,
                          const char *msg, ...);

void __wrap__mtinfo(const char *tag,
                          const char * file,
                          int line,
                          const char * func,
                          const char *msg, ...);

void __wrap__mtwarn(const char *tag,
                          const char * file,
                          int line,
                          const char * func,
                          const char *msg, ...);

void __wrap__mwarn(const char * file,
                   int line,
                   const char * func,
                   const char *msg, ...);

char * __wrap_win_strerror(unsigned long error);

#endif
