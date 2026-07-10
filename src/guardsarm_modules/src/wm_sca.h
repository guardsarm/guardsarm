/*
 * GuardSarm Module for Security Configuration Assessment
 * Copyright (C) 2026 GuardSarm, Inc.
 * November 25, 2018.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef GM_SCA_H
#define GM_SCA_H

#include "os_xml.h"
#include "wmodules_def.h"
#include "schedule_scan.h"

#define GM_SCA_LOGTAG ARGV0 ":sca"

typedef struct gm_sca_policy_t {
    unsigned int enabled:1;
    unsigned int remote:1;
    char *policy_path;
    char *policy_id;
    char *policy_regex_type;
} gm_sca_policy_t;

typedef struct gm_sca_db_sync_flags_t {
    unsigned int enable_synchronization:1;  // Enable database synchronization
    uint32_t sync_interval;                 // Synchronization interval
    uint32_t sync_end_delay;                // Delay for synchronization end message
    uint32_t sync_response_timeout;         // Minimum interval for the synchronization process
    long sync_max_eps;                      // Maximum events per second for synchronization messages.
    uint32_t integrity_interval;            // Integrity check interval (0 = disabled)
} gm_sca_db_sync_flags_t;

typedef struct gm_sca_t {
    int enabled;
    int scan_on_start;
    int max_eps;
    gm_sca_policy_t** policies;
    int remote_commands:1;
    int commands_timeout;
    sched_scan_config scan_config;
    gm_sca_db_sync_flags_t sync;
} gm_sca_t;

extern const gm_context GM_SCA_CONTEXT;

// Read configuration and return a module (if enabled) or NULL (if disabled)
int gm_sca_read(const OS_XML* xml, xml_node** nodes, gmodule* module);

#endif // WM_SCA_H
