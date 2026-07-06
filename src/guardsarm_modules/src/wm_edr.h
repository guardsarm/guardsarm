/*
 * GuardSarm Module for native EDR telemetry (process + network)
 * Copyright (C) 2015, Wazuh Inc.
 *
 * Native on-agent endpoint telemetry collector. Watches /proc for short-lived
 * process executions (with parent lineage + command line + user) and active
 * network connections, and streams each as a JSON event to the manager through
 * the standard agent message queue. Replaces the external Python collector with
 * a first-class agent module compiled into guardsarm-modulesd.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation.
 */

#ifndef WM_EDR_H
#define WM_EDR_H

#include "wmodules_def.h"
#include "os_xml.h"

#define WM_EDR_CONTEXT_NAME  "edr"
#define WM_EDR_LOGTAG        ARGV0 ":edr"
#define WM_EDR_DEFAULT_INTERVAL 5          // seconds between /proc sweeps
#define WM_EDR_LOG_PATH      "logs/edr-telemetry.log"

typedef struct wm_edr_flags_t {
    unsigned int enabled:1;                // main switch
    unsigned int processes:1;              // process-exec telemetry
    unsigned int network:1;                // network-connection telemetry
    unsigned int running:1;                // module is running
} wm_edr_flags_t;

typedef struct wm_edr_t {
    unsigned int interval;                 // sweep interval (seconds)
    wm_edr_flags_t flags;                  // feature switches
    long max_eps;                          // max events per second
} wm_edr_t;

extern const wm_context WM_EDR_CONTEXT;    // module context

// Parse the <wodle name="edr"> configuration block.
int wm_edr_read(const OS_XML *xml, XML_NODE nodes, wmodule *module);

#endif // WM_EDR_H
