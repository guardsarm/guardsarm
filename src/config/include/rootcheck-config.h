/* Copyright (C) 2026 GuardSarm, Inc.
 * Copyright (C) 2009 Trend Micro Inc.
 * All right reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef CROOTCHECK_H
#define CROOTCHECK_H

#include <stdio.h>
#include "os_regex.h"

#define RK_CONF_UNPARSED -2
#define RK_CONF_UNDEFINED -1

typedef struct _rkconfig {
    const char *workdir;
    char *basedir;
    char **ignore;
    OSMatch **ignore_sregex;
    char **alert_msg;

    FILE *fp;
    int daemon;
    int notify; /* QUEUE or SYSLOG */
    int scanall;
    int readall;
    int disabled;
    short skip_nfs;
    int tsleep;

    int time;
    int queue;

    struct _checks {
        short rc_dev;
        short rc_if;
        short rc_pids;
        short rc_ports;
        short rc_sys;
    } checks;

} rkconfig;

/* Frees the Rootcheck struct  */
void Free_Rootcheck(rkconfig * config);

#endif /* CROOTCHECK_H */
