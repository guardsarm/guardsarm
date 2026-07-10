/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */


#ifndef SELECT_WRAPPERS_H
#define SELECT_WRAPPERS_H
#ifndef WIN32

#include <sys/select.h>

int __wrap_select(int nfds,
                  fd_set *restrict readfds,
                  fd_set *restrict writefds,
                  fd_set *restrict errorfds,
                  struct timeval *restrict timeout);

#endif

#endif
