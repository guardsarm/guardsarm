/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */


#ifndef DIRENT_WRAPPERS_MACOS_H
#define DIRENT_WRAPPERS_MACOS_H

#include <dirent.h>

#undef closedir
#define closedir wrap_closedir
#undef opendir
#define opendir wrap_opendir
#undef readdir
#define readdir wrap_readdir

int wrap_closedir(__attribute__((unused)) DIR *dirp);

DIR * wrap_opendir(const char *filename);

struct dirent * wrap_readdir(DIR *dirp);

#endif
