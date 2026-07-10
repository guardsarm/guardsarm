/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */


#ifndef READPROC_WRAPPERS_H
#define READPROC_WRAPPERS_H

#include "../../../../external/procps/readproc.h"

void __wrap_closeproc(PROCTAB* PT);


void __wrap_freeproc(proc_t* p);

PROCTAB* __wrap_openproc(int flags, ...);

proc_t* __wrap_readproc(PROCTAB *restrict const PT,
                        proc_t *restrict p);

#endif
