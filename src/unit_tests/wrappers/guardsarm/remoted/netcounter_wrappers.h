/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef NETCOUNTER_WRAPPERS_H
#define NETCOUNTER_WRAPPERS_H

void __wrap_rem_setCounter(int fd, size_t counter);
size_t __wrap_rem_getCounter(int fd);

#endif
