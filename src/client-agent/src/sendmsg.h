/* Copyright (C) 2026 GuardSarm, Inc.
 * Copyright (C) 2009 Trend Micro Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef AGENTD_SENDMSG_H
#define AGENTD_SENDMSG_H

#include "agentd.h"

/* Send message to server */
int send_msg(const char* msg, ssize_t msg_length);

#endif /* AGENTD_SENDMSG_H */
