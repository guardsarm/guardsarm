/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef STARTUP_GATE_OP_H
#define STARTUP_GATE_OP_H

/**
 * @brief Block a daemon startup until agentd startup hash validation is ready.
 *
 * This function is a no-op when startup hash blocking is disabled.
 *
 * @param module_name Daemon name for logs.
 */
void startup_gate_wait_for_ready(const char *module_name);

#endif /* STARTUP_GATE_OP_H */
