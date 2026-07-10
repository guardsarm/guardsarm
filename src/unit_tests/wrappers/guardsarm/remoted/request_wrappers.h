/*
 * GuardSarm Shared Configuration Manager
 * Copyright (C) 2026 GuardSarm, Inc.
 * April 3, 2018.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef REQUEST_WRAPPERS_H
#define REQUEST_WRAPPERS_H

int __wrap_req_save(const char * counter, const char * buffer, size_t length);

#endif /* REQUEST_WRAPPERS_H */
