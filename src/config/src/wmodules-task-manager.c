/* Copyright (C) 2026 GuardSarm, Inc.
 * All right reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
*/

#include "wmodules.h"

static const char *XML_CLEANUP_TIME = "cleanup_time";
static const char *XML_TASK_TIMEOUT = "task_timeout";

int gm_task_manager_read(__attribute__((unused)) const OS_XML *xml, xml_node **nodes, gmodule *module) {

    unsigned int i;
    gm_task_manager* data;

    if (!module->data) {
        os_calloc(1, sizeof(gm_task_manager), data);
        data->enabled = 1;
        data->cleanup_time = GM_TASK_DEFAULT_CLEANUP_TIME;
        data->task_timeout = GM_TASK_MAX_IN_PROGRESS_TIME;
        module->context = &GM_TASK_MANAGER_CONTEXT;
        module->tag = strdup(module->context->name);
        module->data = data;
    }

    data = module->data;

    if (!nodes) {
        return 0;
    }

    for (i = 0; nodes[i]; i++)
    {
        if(!nodes[i]->element) {
            merror(XML_ELEMNULL);
            return OS_INVALID;
        } else if (!strcmp(nodes[i]->element, XML_CLEANUP_TIME)) {
            char *endptr;
            data->cleanup_time = strtol(nodes[i]->content, &endptr, 0);

            if (data->cleanup_time == 0 || data->cleanup_time == INT_MAX) {
                merror("Invalid cleanup_time at module '%s'", GM_TASK_MANAGER_CONTEXT.name);
                return OS_INVALID;
            }

            switch (*endptr) {
            case 'd':
                data->cleanup_time *= 86400;
                break;
            case 'h':
                data->cleanup_time *= 3600;
                break;
            case 'm':
                data->cleanup_time *= 60;
                break;
            case 's':
            case '\0':
                break;
            default:
                merror("Invalid cleanup_time at module '%s'", GM_TASK_MANAGER_CONTEXT.name);
                return OS_INVALID;
            }
        } else if (!strcmp(nodes[i]->element, XML_TASK_TIMEOUT)) {
            char *endptr;
            data->task_timeout = strtol(nodes[i]->content, &endptr, 0);

            if (data->task_timeout == 0 || data->task_timeout == INT_MAX) {
                merror("Invalid task_timeout at module '%s'", GM_TASK_MANAGER_CONTEXT.name);
                return OS_INVALID;
            }

            switch (*endptr) {
            case 'd':
                data->task_timeout *= 86400;
                break;
            case 'h':
                data->task_timeout *= 3600;
                break;
            case 'm':
                data->task_timeout *= 60;
                break;
            case 's':
            case '\0':
                break;
            default:
                merror("Invalid task_timeout at module '%s'", GM_TASK_MANAGER_CONTEXT.name);
                return OS_INVALID;
            }
        } else {
            mwarn("No such tag <%s> at module '%s'.", nodes[i]->element, GM_TASK_MANAGER_CONTEXT.name);
        }
    }

    if (data->cleanup_time < data->task_timeout) {
        merror("Too short cleanup_time at module '%s'", GM_TASK_MANAGER_CONTEXT.name);
        return OS_INVALID;
    }

    return 0;
}
