/* Copyright (C) 2026 GuardSarm, Inc.
 * All right reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef CCONFIG_H
#define CCONFIG_H

#include "shared.h"

/* Configuration structure */
typedef struct __Config {
    /* Agent's disconnection global parameters */
    long agents_disconnection_time;
    long agents_disconnection_alert_time;
} _Config;

#endif /* CCONFIG_H */
