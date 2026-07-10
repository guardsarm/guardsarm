/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */


#ifndef MONITORD_WRAPPERS_H
#define MONITORD_WRAPPERS_H

void __wrap_w_rotate_log(int compress, int keep_log_days, int new_day, int rotate_json, int daily_rotations);

#endif
