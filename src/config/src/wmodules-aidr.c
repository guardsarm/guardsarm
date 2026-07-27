/*
 * GuardSarm Module Config for native AIDR (AI Detection & Response) telemetry
 * Copyright (C) 2026 GuardSarm, Inc.
 *
 * Parses the <wodle name="aidr"> block: enable/disable, sweep interval, the
 * per-collector feature switches (runtimes / api_calls / mcp / models), the
 * event-rate cap and the model-hash size cap.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized
 * copying, distribution, modification, or use is prohibited except under a
 * written license agreement with GuardSarm, Inc.
 */

#include "wmodules.h"

// The native AIDR module has both a Linux (/proc) and a Windows (Win32)
// implementation (see wm_aidr.c). The <wodle name="aidr"> config reader is
// agent-only (CLIENT) and platform-independent, so it is built for both the
// Linux and Windows agents.
#if defined(CLIENT)

static const char *XML_DISABLED = "disabled";
static const char *XML_INTERVAL = "interval";
static const char *XML_RUNTIMES = "runtimes";
static const char *XML_API_CALLS = "api_calls";
static const char *XML_MCP = "mcp";
static const char *XML_MODELS = "models";
static const char *XML_MAX_EPS = "max_eps";
static const char *XML_MAX_HASH_MB = "max_hash_mb";

int gm_aidr_read(const OS_XML *xml, XML_NODE node, gmodule *module) {
    gm_aidr_t *aidr;
    int i;

    (void)xml;

    if (!module->data) {
        os_calloc(1, sizeof(gm_aidr_t), aidr);
        aidr->flags.enabled = 1;
        aidr->flags.runtimes = 1;
        aidr->flags.api_calls = 1;
        aidr->flags.mcp = 1;
        aidr->flags.models = 1;
        aidr->interval = GM_AIDR_DEFAULT_INTERVAL;
        aidr->max_eps = 50;
        aidr->max_hash_mb = GM_AIDR_DEFAULT_HASH_MB;
        module->context = &GM_AIDR_CONTEXT;
        module->tag = strdup(module->context->name);
        module->data = aidr;
    }

    aidr = module->data;

    if (!node) {
        return 0;
    }

    for (i = 0; node[i]; i++) {
        if (!node[i]->element) {
            merror("Element NULL at module '%s'.", GM_AIDR_CONTEXT.name);
            return OS_INVALID;
        } else if (!strcmp(node[i]->element, XML_DISABLED)) {
            if (node[i]->content && !strcmp(node[i]->content, "yes")) {
                aidr->flags.enabled = 0;
            } else if (node[i]->content && !strcmp(node[i]->content, "no")) {
                aidr->flags.enabled = 1;
            } else {
                merror("Invalid content for tag '%s' at module '%s'.", XML_DISABLED, GM_AIDR_CONTEXT.name);
                return OS_INVALID;
            }
        } else if (!strcmp(node[i]->element, XML_INTERVAL)) {
            if (!node[i]->content || !strlen(node[i]->content)) {
                merror("Invalid interval at module '%s'.", GM_AIDR_CONTEXT.name);
                return OS_INVALID;
            }
            char *endptr;
            aidr->interval = strtoul(node[i]->content, &endptr, 0);
            switch (*endptr) {
                case 'd': aidr->interval *= 86400; break;
                case 'h': aidr->interval *= 3600; break;
                case 'm': aidr->interval *= 60; break;
                case 's': case '\0': break;
                default:
                    merror("Invalid interval at module '%s'.", GM_AIDR_CONTEXT.name);
                    return OS_INVALID;
            }
            if (aidr->interval == 0) {
                aidr->interval = GM_AIDR_DEFAULT_INTERVAL;
            }
        } else if (!strcmp(node[i]->element, XML_RUNTIMES)) {
            aidr->flags.runtimes = (node[i]->content && !strcmp(node[i]->content, "yes")) ? 1 : 0;
        } else if (!strcmp(node[i]->element, XML_API_CALLS)) {
            aidr->flags.api_calls = (node[i]->content && !strcmp(node[i]->content, "yes")) ? 1 : 0;
        } else if (!strcmp(node[i]->element, XML_MCP)) {
            aidr->flags.mcp = (node[i]->content && !strcmp(node[i]->content, "yes")) ? 1 : 0;
        } else if (!strcmp(node[i]->element, XML_MODELS)) {
            aidr->flags.models = (node[i]->content && !strcmp(node[i]->content, "yes")) ? 1 : 0;
        } else if (!strcmp(node[i]->element, XML_MAX_EPS)) {
            if (node[i]->content) {
                aidr->max_eps = atol(node[i]->content);
            }
        } else if (!strcmp(node[i]->element, XML_MAX_HASH_MB)) {
            if (node[i]->content) {
                aidr->max_hash_mb = atol(node[i]->content);
            }
        } else {
            merror("No such tag '%s' at module '%s'.", node[i]->element, GM_AIDR_CONTEXT.name);
            return OS_INVALID;
        }
    }

    return 0;
}

#endif  // CLIENT
