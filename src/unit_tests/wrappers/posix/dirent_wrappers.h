/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */


#ifndef DIRENT_WRAPPERS_H
#define DIRENT_WRAPPERS_H

#include <dirent.h>

int __wrap_closedir(DIR *dirp);

DIR * __wrap_opendir(const char *name);

struct dirent * __wrap_readdir(DIR *dirp);

#endif
