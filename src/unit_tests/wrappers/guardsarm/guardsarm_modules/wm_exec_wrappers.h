/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */


#ifndef WM_EXEC_WRAPPERS_H
#define WM_EXEC_WRAPPERS_H

int __wrap_wm_exec(char *command, char **output, int *exitcode, int secs, const char * add_path);
void expect_wm_exec(char *command, int sec, const char * add_path, char *output_command, int exitcode, int return_code);

#endif
