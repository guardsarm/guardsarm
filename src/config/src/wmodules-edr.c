/*
 * GuardSarm Module Config for native EDR telemetry
 * Copyright (C) 2026 GuardSarm, Inc.
 *
 * Parses the <wodle name="edr"> block: enable/disable, sweep interval, and the
 * process / network feature switches.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#include "wmodules.h"

// The native EDR module has both a Linux (/proc) and a Windows (Win32) implementation
// (see wm_edr.c). The <wodle name="edr"> config reader is agent-only (CLIENT) and
// platform-independent, so it is built for both the Linux and Windows agents.
#if defined(CLIENT)

static const char *XML_DISABLED = "disabled";
static const char *XML_INTERVAL = "interval";
static const char *XML_PROCESSES = "processes";
static const char *XML_NETWORK = "network";
static const char *XML_PERSISTENCE = "persistence";
static const char *XML_MAX_EPS = "max_eps";

int gm_edr_read(const OS_XML *xml, XML_NODE node, gmodule *module) {
    gm_edr_t *edr;
    int i;

    (void)xml;

    if (!module->data) {
        os_calloc(1, sizeof(gm_edr_t), edr);
        edr->flags.enabled = 1;
        edr->flags.processes = 1;
        edr->flags.network = 1;
        edr->flags.persistence = 1;
        edr->interval = GM_EDR_DEFAULT_INTERVAL;
        edr->max_eps = 100;
        module->context = &GM_EDR_CONTEXT;
        module->tag = strdup(module->context->name);
        module->data = edr;
    }

    edr = module->data;

    if (!node) {
        return 0;
    }

    for (i = 0; node[i]; i++) {
        if (!node[i]->element) {
            merror("Element NULL at module '%s'.", GM_EDR_CONTEXT.name);
            return OS_INVALID;
        } else if (!strcmp(node[i]->element, XML_DISABLED)) {
            if (node[i]->content && !strcmp(node[i]->content, "yes")) {
                edr->flags.enabled = 0;
            } else if (node[i]->content && !strcmp(node[i]->content, "no")) {
                edr->flags.enabled = 1;
            } else {
                merror("Invalid content for tag '%s' at module '%s'.", XML_DISABLED, GM_EDR_CONTEXT.name);
                return OS_INVALID;
            }
        } else if (!strcmp(node[i]->element, XML_INTERVAL)) {
            if (!node[i]->content || !strlen(node[i]->content)) {
                merror("Invalid interval at module '%s'.", GM_EDR_CONTEXT.name);
                return OS_INVALID;
            }
            char *endptr;
            edr->interval = strtoul(node[i]->content, &endptr, 0);
            switch (*endptr) {
                case 'd': edr->interval *= 86400; break;
                case 'h': edr->interval *= 3600; break;
                case 'm': edr->interval *= 60; break;
                case 's': case '\0': break;
                default:
                    merror("Invalid interval at module '%s'.", GM_EDR_CONTEXT.name);
                    return OS_INVALID;
            }
            if (edr->interval == 0) {
                edr->interval = GM_EDR_DEFAULT_INTERVAL;
            }
        } else if (!strcmp(node[i]->element, XML_PROCESSES)) {
            edr->flags.processes = (node[i]->content && !strcmp(node[i]->content, "yes")) ? 1 : 0;
        } else if (!strcmp(node[i]->element, XML_NETWORK)) {
            edr->flags.network = (node[i]->content && !strcmp(node[i]->content, "yes")) ? 1 : 0;
        } else if (!strcmp(node[i]->element, XML_PERSISTENCE)) {
            edr->flags.persistence = (node[i]->content && !strcmp(node[i]->content, "yes")) ? 1 : 0;
        } else if (!strcmp(node[i]->element, XML_MAX_EPS)) {
            if (node[i]->content) {
                edr->max_eps = atol(node[i]->content);
            }
        } else {
            merror("No such tag '%s' at module '%s'.", node[i]->element, GM_EDR_CONTEXT.name);
            return OS_INVALID;
        }
    }

    return 0;
}

#endif  // CLIENT
