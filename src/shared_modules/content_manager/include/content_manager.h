/*
 * GuardSarm content manager.
 * Copyright (C) 2026 GuardSarm, Inc.
 * May 1, 2023.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef _CONTENT_MANAGER_H
#define _CONTENT_MANAGER_H

// Define EXPORTED for any platform

#if __GNUC__ >= 4
#define EXPORTED __attribute__((visibility("default")))
#else
#define EXPORTED
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#include "commonDefs.h"

    EXPORTED void content_manager_start(full_log_fnc_t callbackLog);

    EXPORTED void content_manager_stop();

#ifdef __cplusplus
}
#endif

typedef void (*content_manager_start_func)(full_log_fnc_t callbackLog);

typedef void (*content_manager_stop_func)();

#endif // _CONTENT_MANAGER_H
