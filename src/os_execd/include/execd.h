/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef EXECD_H
#define EXECD_H

#ifndef ARGV0
#define ARGV0 "guardsarm-execd"
#endif

/* Arguments for the commands */
#define ENABLE_ENTRY       "enable"
#define DISABLE_ENTRY      "disable"
#define CONTINUE_ENTRY     "continue"
#define ABORT_ENTRY        "abort"

/* Maximum number of active responses active */
#define MAX_AR      64

/* Maximum number of command arguments */
#define MAX_ARGS    32

/* Execd select timeout -- in seconds */
#define EXECD_TIMEOUT   1

extern int repeated_offenders_timeout[];
extern time_t pending_upg;
extern int is_disabled;
extern int req_timeout;
extern int max_restart_lock;

/** Function prototypes **/

cJSON *getARConfig(void);
cJSON *getExecdInternalOptions(void);
cJSON *getClusterConfig(void);
void ExecCmd(char *const *cmd) __attribute__((nonnull));
void ExecCmd_Win32(char *cmd);
int ExecdConfig(const char *cfgfile) __attribute__((nonnull));

#ifdef WIN32
int WinExecdStart(void);
void ExecdRun(char *exec_msg);
void ExecdTimeoutRun();
void ExecdShutdown();
#else
#ifdef GUARDSARM_UNIT_TESTING
void ExecdStart(int q);
#else
void ExecdStart(int q) __attribute__((noreturn));
#endif
void ExecdRun(char *exec_msg, int *childcount);
void ExecdTimeoutRun(int *childcount);
void ExecdShutdown(int sig) __attribute__((noreturn));
#endif

size_t wcom_unmerge(const char *file_path, char **output);
size_t wcom_uncompress(const char * source, const char * target, char ** output);
size_t wcom_dispatch(char *command, char **output);
size_t lock_restart(int timeout);
size_t wcom_getconfig(const char * section, char ** output);
size_t wcom_check_manager_config(char **output);

#ifndef WIN32
// Com request thread dispatcher
void * wcom_main(void * arg);
#endif

/* Timeout data structure */
typedef struct _timeout_data {
    time_t time_of_addition;
    int time_to_block;
    char **command;
    char *parameters;
    char *rkey;
} timeout_data;

void FreeTimeoutEntry(timeout_data *timeout_entry);
void FreeTimeoutList();

#endif /* EXECD_H */
