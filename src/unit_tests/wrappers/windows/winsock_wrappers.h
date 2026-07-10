/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */


#ifndef WINSOCK_WRAPPERS_H
#define WINSOCK_WRAPPERS_H

#include <winsock.h>
#define gethostname wrap_gethostname

int wrap_gethostname(char *name, int len);

#endif
