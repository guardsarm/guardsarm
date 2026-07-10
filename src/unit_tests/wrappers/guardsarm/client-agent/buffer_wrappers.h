/*
 * Copyright (C) 2026 GuardSarm, Inc.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef BUFFER_WRAPPERS_H
#define BUFFER_WRAPPERS_H

#include <pthread.h>
#include "shared.h"
#include "agentd.h"

#ifdef WIN32
#include <winsock2.h>
#include <windows.h>
#endif

int __wrap_w_agentd_get_buffer_lenght();

#endif
