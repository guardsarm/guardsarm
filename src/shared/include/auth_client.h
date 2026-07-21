/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef AUTH_CLIENT_H
#define AUTH_CLIENT_H

// Remove agent. Returns 0 on success or -1 on error.
int auth_remove_agent(int sock, const char *id, int json_format);

#endif /* AUTH_CLIENT_H */
