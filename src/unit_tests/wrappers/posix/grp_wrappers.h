/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */


#ifndef GRP_WRAPPERS_H
#define GRP_WRAPPERS_H

#ifndef WIN32
#include <stddef.h>
#include <sys/types.h>
#include <grp.h>
#include <errno.h>


struct group *__wrap_getgrgid(gid_t gid);

int __wrap_getgrnam_r(const char *name, struct group *grp, char *buf, size_t buflen, struct group **result);

#endif
#endif
