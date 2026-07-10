/*
 * GuardSarm Module for System inventory
 * Copyright (C) 2026 GuardSarm, Inc.
 * November 17, 2020.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#include "wmodules_def.h"
#include "os_xml.h"

#ifndef GM_SYSCOLLECTOR
#define GM_SYSCOLLECTOR

extern const gm_context GM_SYS_CONTEXT;     // Context

#define GM_SYS_LOGTAG ARGV0 ":syscollector" // Tag for log messages
#define GM_SYSCOLLECTOR_DEFAULT_INTERVAL W_HOUR_SECONDS

typedef struct gm_sys_flags_t {
    unsigned int enabled:1;                 // Main switch
    unsigned int scan_on_start:1;           // Scan always on start
    unsigned int notify_first_scan:1;       // Notify the first scan
    unsigned int hwinfo:1;                  // Hardware inventory
    unsigned int netinfo:1;                 // Network inventory
    unsigned int osinfo:1;                  // OS inventory
    unsigned int programinfo:1;             // Installed packages inventory
    unsigned int hotfixinfo:1;              // Windows hotfixes installed
    unsigned int portsinfo:1;               // Opened ports inventory
    unsigned int allports:1;                // Scan only listening ports or all
    unsigned int procinfo:1;                // Running processes inventory
    unsigned int running:1;                 // The module is running
    unsigned int groups:1;                  // Groups inventory
    unsigned int users:1;                   // Users inventory
    unsigned int services:1;                // Services inventory
    unsigned int browser_extensions:1;      // Browser extensions inventory
} gm_sys_flags_t;

typedef struct gm_sys_state_t {
    time_t next_time;                       // Absolute time for next scan
} gm_sys_state_t;

typedef struct gm_sys_db_sync_flags_t {
    unsigned int enable_synchronization:1;  // Enable database synchronization
    uint32_t sync_interval;                 // Synchronization interval
    uint32_t sync_end_delay;                // Delay for synchronization end message
    uint32_t sync_response_timeout;         // Minimum interval for the synchronization process
    long sync_max_eps;                      // Maximum events per second for synchronization messages.
    uint32_t integrity_interval;            // Integrity check interval (0 = disabled)
} gm_sys_db_sync_flags_t;

typedef struct gm_sys_t {
    unsigned int interval;                  // Time interval between cycles (seconds)
    gm_sys_flags_t flags;                   // Flag bitfield
    gm_sys_state_t state;                   // Running state
    gm_sys_db_sync_flags_t sync;            // Database synchronization value
    int max_eps;                            // Maximum events per second.
} gm_sys_t;

// Parse XML configuration
int gm_syscollector_read(const OS_XML *xml, XML_NODE node, gmodule *module);

// Query function type for agentd communication (cross-platform)
// Fills output_buffer with JSON response on success
// Returns true on success, false on error
typedef bool (*agentd_query_func_t)(const char* command, char* output_buffer, size_t buffer_size);

// Query agentd with a command and fill output_buffer with JSON response
// Returns true on success (output_buffer contains JSON), false on error
// Works on both Unix/Linux (socket) and Windows (agcom_dispatch)
bool gm_sys_query_agentd(const char* command, char* output_buffer, size_t buffer_size);

// Set agentd query function (must be called before syscollector_start)
void syscollector_set_agentd_query_func(agentd_query_func_t queryFunc);

#endif
