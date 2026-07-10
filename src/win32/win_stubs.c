/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

/*
 * Stub implementations for Windows-specific functions that are called
 * from shared library code but are only meaningful in the agent context.
 */

#ifdef WIN32

#include "shared.h"

/* Stub for WinSetError */
void WinSetError() {
    /* Active-response executables don't need this */
}

#endif /* WIN32 */
