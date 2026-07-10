/*
 * GuardSarm Module for Agent control
 * Copyright (C) 2026 GuardSarm, Inc.
 * January, 2019
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */
#ifndef WIN32
#ifndef GM_CONTROL
#define GM_CONTROL

#define GM_CONTROL_LOGTAG ARGV0 ":control"

#include "wmodules.h"

extern const gm_context GM_CONTROL_CONTEXT;

typedef struct gm_control_t {
    unsigned int enabled:1;
    unsigned int run_on_start:1;
} gm_control_t;

gmodule *gm_control_read();

/**
 * @brief Dispatch control commands and execute corresponding actions
 *
 * Parses incoming commands and routes them to appropriate handlers.
 * On manager builds, acts on "guardsarm-manager"; on agent builds, on "guardsarm-agent".
 * Supported commands: restart, reload
 *
 * @param command Command string with optional arguments
 * @param output Pointer to string that will contain the response message
 * @return size_t Length of the output string
 */
size_t gm_control_dispatch(char *command, char **output);

/**
 * @brief Check if systemd is available as the init system
 * @return true if systemd is available, false otherwise
 */
bool gm_control_check_systemd();

/**
 * @brief Wait for a GuardSarm service to be in active state
 *
 * Waits up to 60 seconds for the service to become active.
 *
 * @param service Service name to check (e.g. "guardsarm-manager", "guardsarm-agent")
 * @return true if service is active, false otherwise
 */
bool gm_control_wait_for_service_active(const char *service);

/**
 * @brief Get the control binary path used by the direct-exec fallback
 *
 * Selected at compile time: "bin/guardsarm-control" on agent builds (CLIENT),
 * "bin/guardsarm-manager-control" on manager builds. The path is relative to the
 * install directory (modulesd chdir()s there at startup).
 *
 * @return const char* Relative path to the control binary
 */
const char *gm_control_get_bin(void);

/**
 * @brief Execute restart or reload action on a GuardSarm service
 *
 * Detects if systemd is available and uses systemctl, otherwise falls back to the
 * control binary (see wm_control_get_bin()).
 * For reload actions, waits for service to be active before proceeding (systemd only).
 *
 * @param action "restart" or "reload"
 * @param service Service name (e.g. "guardsarm-manager", "guardsarm-agent")
 * @param output Pointer to string that will contain the response message
 * @return size_t Length of the output string
 */
size_t gm_control_execute_action(const char *action, const char *service, char **output);

#endif
#endif
