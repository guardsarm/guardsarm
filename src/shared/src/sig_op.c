/* Copyright (C) 2026 GuardSarm, Inc.
 * All right reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

/* Functions to handle signal manipulation */

#ifndef WIN32

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

#include "shared.h"
#include "sig_op.h"
#include "file_op.h"
#include "debug_op.h"
#include "error_messages.h"
#include "debug_messages.h"

static const char *pidfile = NULL;

void HandleExit() {
    DeletePID(pidfile);

    if (strcmp(__local_name, "unset")) {
        DeleteState();
    }
}

void HandleSIG(int sig)
{
    minfo(SIGNAL_RECV, sig, strsignal(sig));

    exit(1);
}


/* To avoid client-server communication problems */
void HandleSIGPIPE(__attribute__((unused)) int sig)
{
    return;
}

void StartSIG(const char *process_name)
{
    pidfile = process_name;

    signal(SIGHUP, SIG_IGN);
    signal(SIGINT, HandleSIG);
    signal(SIGQUIT, HandleSIG);
    signal(SIGTERM, HandleSIG);
    signal(SIGALRM, HandleSIG);
    signal(SIGPIPE, HandleSIGPIPE);

    atexit(HandleExit);
}

void StartSIG2(const char *process_name, void (*func)(int))
{
    pidfile = process_name;

    signal(SIGHUP, SIG_IGN);
    signal(SIGINT, func);
    signal(SIGQUIT, func);
    signal(SIGTERM, func);
    signal(SIGALRM, func);
    signal(SIGPIPE, HandleSIGPIPE);

    atexit(HandleExit);
}

#endif /* !WIN32 */
