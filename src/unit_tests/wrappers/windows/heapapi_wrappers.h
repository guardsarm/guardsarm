
/*
 * Copyright (C) 2026 GuardSarm, Inc.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */
#ifndef HEAPAPI_WRAPPERS_H
#define HEAPAPI_WRAPPERS_H

#include <windows.h>

#undef win_alloc
#define win_alloc wrap_win_alloc

LPVOID wrap_win_alloc(SIZE_T size);

#endif
