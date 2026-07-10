/*
 * GuardSarm Module for Docker
 * Copyright (C) 2026 GuardSarm, Inc.
 * October, 2018.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef WM_DOCKER
#define WM_DOCKER
#ifndef WIN32

#define WM_DOCKER_LOGTAG ARGV0 ":docker-listener"
#define WM_DOCKER_SCRIPT_PATH  "wodles/docker/DockerListener"

#define WM_DOCKER_DEF_INTERVAL 60

typedef struct wm_docker_flags_t {
    unsigned int enabled:1;
    unsigned int run_on_start:1;
} wm_docker_flags_t;

typedef struct wm_docker_t {
    unsigned int interval;             // Time interval to retry to run the listener
    int attempts;                      // Maximum attempts to run the module after fails
    wm_docker_flags_t flags;           // Default flags
    sched_scan_config scan_config;
} wm_docker_t;

extern const wm_context WM_DOCKER_CONTEXT;   // Context

// Parse XML configuration
int wm_docker_read(xml_node **nodes, wmodule *module);

#endif
#endif
