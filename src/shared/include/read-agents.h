/* Copyright (C) 2026 GuardSarm, Inc.
 * All right reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef CRAGENT_H
#define CRAGENT_H

#include <cJSON.h>

/* Status */
typedef enum agent_status_t {
    GA_STATUS_ACTIVE = 12,
    GA_STATUS_NACTIVE,
    GA_STATUS_NEVER,
    GA_STATUS_PENDING,
    GA_STATUS_UNKNOWN
} agent_status_t;

/* Delete diff folders */
void delete_diff(const char *name);

#ifndef WIN32
/* Return the unix permission string
 * Returns a pointer to a local static array
 */
char *agent_file_perm(mode_t mode);
#endif

#endif /* CRAGENT_H */
