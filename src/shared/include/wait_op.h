/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef WAIT_OP_H
#define WAIT_OP_H

void os_setwait(void);
void os_delwait(void);
void os_wait(void);
void os_wait_predicate(bool (*fn_ptr)());

/**
 * @brief Check whether the agent wait mark is on (manager is disconnected)
 *
 * @retval true The agent is blocked.
 * @retval false The agent is not blocked.
 */
bool os_iswait();

#endif /* WAIT_OP_H */
