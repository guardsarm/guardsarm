/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef GM_AGENT_UPGRADE_AGENT_WRAPPERS_H
#define GM_AGENT_UPGRADE_AGENT_WRAPPERS_H

#include "shared.h"
#include "wm_agent_upgrade_agent.h"
#include "wmodules.h"

void __wrap_wm_agent_upgrade_start_agent_module(const gm_agent_configs* agent_config, const int enabled);

size_t __wrap_wm_agent_upgrade_process_command(const char* buffer, char** output);

#endif
