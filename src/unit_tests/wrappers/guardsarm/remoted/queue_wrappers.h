/*
 * GuardSarm Shared Configuration Manager
 * Copyright (C) 2026 GuardSarm, Inc.
 * May 17, 2022.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef REM_QUEUE_WRAPPERS_H
#define REM_QUEUE_WRAPPERS_H

#include <stddef.h>

size_t __wrap_rem_get_qsize();
size_t __wrap_rem_get_tsize();

int __wrap_rem_msgpush(const char * buffer, unsigned long size, struct sockaddr_storage * addr, int sock);

#endif /* REM_QUEUE_WRAPPERS_H */
