/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */


#ifndef AUTH_CLIENT_WRAPPERS_H
#define AUTH_CLIENT_WRAPPERS_H

int __wrap_auth_remove_agent(__attribute__((unused)) int sock, const char *id, __attribute__((unused)) int json_format);

#endif
