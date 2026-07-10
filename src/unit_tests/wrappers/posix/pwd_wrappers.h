/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */


#ifndef PWD_WRAPPERS_H
#define PWD_WRAPPERS_H

#ifndef WIN32
#include <stddef.h>
#include <pwd.h>

int __wrap_getpwnam_r(const char *name,
                      struct passwd *pwd,
                      char *buf,
                      size_t buflen,
                      struct passwd **result);
int __wrap_getpwuid_r(uid_t uid,
                      struct passwd *pwd,
                      char *buf,
                      size_t buflen,
                      struct passwd **result);
#endif
#endif
