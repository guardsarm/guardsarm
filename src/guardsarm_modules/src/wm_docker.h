/*
 * GuardSarm Module for Docker
 * Copyright (C) 2026 GuardSarm, Inc.
 * October, 2018.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef GM_DOCKER
#define GM_DOCKER
#ifndef WIN32

#define GM_DOCKER_LOGTAG ARGV0 ":docker-listener"
#define GM_DOCKER_SCRIPT_PATH  "wodles/docker/DockerListener"

#define GM_DOCKER_DEF_INTERVAL 60

typedef struct gm_docker_flags_t {
    unsigned int enabled:1;
    unsigned int run_on_start:1;
} gm_docker_flags_t;

typedef struct gm_docker_t {
    unsigned int interval;             // Time interval to retry to run the listener
    int attempts;                      // Maximum attempts to run the module after fails
    gm_docker_flags_t flags;           // Default flags
    sched_scan_config scan_config;
} gm_docker_t;

extern const gm_context GM_DOCKER_CONTEXT;   // Context

// Parse XML configuration
int gm_docker_read(xml_node **nodes, gmodule *module);

#endif
#endif
