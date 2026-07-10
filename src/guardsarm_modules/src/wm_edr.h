/*
 * GuardSarm Module for native EDR telemetry (process + network)
 * Copyright (C) 2026 GuardSarm, Inc.
 *
 * Native on-agent endpoint telemetry collector. Watches /proc for short-lived
 * process executions (with parent lineage + command line + user) and active
 * network connections, and streams each as a JSON event to the manager through
 * the standard agent message queue. Replaces the external Python collector with
 * a first-class agent module compiled into guardsarm-modulesd.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef GM_EDR_H
#define GM_EDR_H

#include "wmodules_def.h"
#include "os_xml.h"

#define GM_EDR_CONTEXT_NAME  "edr"
#define GM_EDR_LOGTAG        ARGV0 ":edr"
#define GM_EDR_DEFAULT_INTERVAL 5          // seconds between /proc sweeps
#define GM_EDR_LOG_PATH      "logs/edr-telemetry.log"

typedef struct gm_edr_flags_t {
    unsigned int enabled:1;                // main switch
    unsigned int processes:1;              // process-exec telemetry (+ hash/cwd/script)
    unsigned int network:1;                // network-connection telemetry
    unsigned int persistence:1;            // cron/systemd/startup persistence telemetry
    unsigned int running:1;                // module is running
} gm_edr_flags_t;

typedef struct gm_edr_t {
    unsigned int interval;                 // sweep interval (seconds)
    gm_edr_flags_t flags;                  // feature switches
    long max_eps;                          // max events per second
} gm_edr_t;

extern const gm_context GM_EDR_CONTEXT;    // module context

// Parse the <wodle name="edr"> configuration block.
int gm_edr_read(const OS_XML *xml, XML_NODE nodes, gmodule *module);

#endif // WM_EDR_H
