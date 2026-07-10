/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */


#ifndef READ_AGENTS_WRAPPERS_H
#define READ_AGENTS_WRAPPERS_H

int __wrap_connect_to_remoted();

int __wrap_send_msg_to_agent(int msocket, const char *msg, const char *agt_id, const char *exec);

#endif
