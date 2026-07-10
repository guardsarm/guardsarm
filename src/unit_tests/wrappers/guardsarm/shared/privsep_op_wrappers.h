/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */


#ifndef PRIVSEP_OP_WRAPPERS_H
#define PRIVSEP_OP_WRAPPERS_H

#ifndef WIN32
#include <pwd.h>

struct group *__wrap_w_getgrgid(gid_t gid, struct group *grp,  char *buf, int buflen);
#endif

int __wrap_Privsep_GetUser(const char *name);

int __wrap_Privsep_GetGroup(const char *name);

#endif
