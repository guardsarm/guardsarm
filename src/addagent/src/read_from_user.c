/* Copyright (C) 2026 GuardSarm, Inc.
 * Copyright (C) 2009 Trend Micro Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#include "manage_agents.h"

/* Global variables */
static char __user_buffer[USER_SIZE + 1];
static char *__user_buffer_pt;


char *read_from_user()
{
    memset(__user_buffer, '\0', USER_SIZE + 1);

    if ((fgets(__user_buffer, USER_SIZE - 1, stdin) == NULL) ||
            (strlen(__user_buffer) >= (USER_SIZE - 2))) {
        printf(INPUT_LARGE);
        exit(1);
    }

    __user_buffer_pt = chomp(__user_buffer);

    return (__user_buffer_pt);
}

