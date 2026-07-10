/*
 * GuardSarm Module Configuration
 * Copyright (C) 2026 GuardSarm, Inc.
 * October 26, 2017.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#include "wmodules.h"

static const char *XML_DISABLED = "disabled";
static const char *XML_TAG = "tag";
static const char *XML_COMMAND = "command";
static const char *XML_IGNORE_OUTPUT = "ignore_output";
static const char *XML_RUN_ON_START = "run_on_start";
static const char *XML_TIMEOUT = "timeout";
static const char *XML_VERIFY_MD5 = "verify_md5";
static const char *XML_VERIFY_SHA1 = "verify_sha1";
static const char *XML_VERIFY_SHA256 = "verify_sha256";
static const char *XML_SKIP_VERIFICATION = "skip_verification";

// Parse XML

int gm_command_read(xml_node **nodes, gmodule *module, int agent_cfg)
{

    int i;
    int empty = 0;
    gm_command_t * command;
    size_t command_tag_length;
    char * command_tag = NULL;

    if (!nodes) {
        merror("Tag <%s> not found at module '%s'.", XML_COMMAND, GM_COMMAND_CONTEXT.name);
        return OS_INVALID;
    }

    // Create module

    os_calloc(1, sizeof(gm_command_t), command);
    command->enabled = 1;
    command->run_on_start = 1;
    command->skip_verification = 0;
    sched_scan_init(&(command->scan_config));
    command->scan_config.interval = GM_COMMAND_DEFAULT_INTERVAL;
    command->agent_cfg = agent_cfg;
    command->md5_hash = NULL;
    command->sha1_hash = NULL;
    command->sha256_hash = NULL;
    command->full_command = NULL;
    module->context = &GM_COMMAND_CONTEXT;
    module->data = command;

    // Iterate over module subelements

    for (i = 0; nodes[i]; i++){
        if (!nodes[i]->element) {
            merror(XML_ELEMNULL);
            return OS_INVALID;
        } else if (!strcmp(nodes[i]->element, XML_DISABLED)) {
            if (!strcmp(nodes[i]->content, "yes"))
                command->enabled = 0;
            else if (!strcmp(nodes[i]->content, "no"))
                command->enabled = 1;
            else {
                merror("Invalid content for tag '%s' at module '%s'.", XML_DISABLED, GM_COMMAND_CONTEXT.name);
                return OS_INVALID;
            }
        } else if (!strcmp(nodes[i]->element, XML_TAG)) {
            if (strlen(nodes[i]->content) == 0) {
                mwarn("Empty content for tag '%s' at module '%s'.", XML_TAG, GM_COMMAND_CONTEXT.name);
                command_tag_length = strlen(GM_COMMAND_CONTEXT.name) + 2;
                os_malloc(sizeof(char) * command_tag_length, command_tag);
                snprintf(command_tag, command_tag_length, "%s", GM_COMMAND_CONTEXT.name);
                empty = 1;
            }

            free(command->tag);
            os_strdup(nodes[i]->content, command->tag);

            if (!empty) {
                command_tag_length = strlen(GM_COMMAND_CONTEXT.name) + strlen(command->tag) + 2;
                os_malloc(sizeof(char) * command_tag_length, command_tag);
                snprintf(command_tag, command_tag_length, "%s:%s", GM_COMMAND_CONTEXT.name, command->tag);
            }

            free(module->tag);
            os_strdup(command_tag, module->tag);
            free(command_tag);
            empty = 0;
        } else if (!strcmp(nodes[i]->element, XML_COMMAND)) {
            if (strlen(nodes[i]->content) == 0) {
                merror("Empty content for tag '%s' at module '%s'.", XML_COMMAND, GM_COMMAND_CONTEXT.name);
                return OS_INVALID;
            }
#ifdef WIN32
            if (is_network_path(nodes[i]->content)) {
                merror(NETWORK_PATH_CONFIGURED, nodes[i]->element, nodes[i]->content);
                command->enabled = 0;
                return 0;
            }
#endif
            free(command->command);
            os_strdup(nodes[i]->content, command->command);
        } else if (!strcmp(nodes[i]->element, XML_RUN_ON_START)) {
            if (!strcmp(nodes[i]->content, "yes"))
                command->run_on_start = 1;
            else if (!strcmp(nodes[i]->content, "no"))
                command->run_on_start = 0;
            else {
                merror("Invalid content for tag '%s' at module '%s'.", XML_RUN_ON_START, GM_COMMAND_CONTEXT.name);
                return OS_INVALID;
            }
        } else if (!strcmp(nodes[i]->element, XML_IGNORE_OUTPUT)) {
            if (!strcmp(nodes[i]->content, "yes"))
                command->ignore_output = 1;
            else if (!strcmp(nodes[i]->content, "no"))
                command->ignore_output = 0;
            else {
                merror("Invalid content for tag '%s' at module '%s'.", XML_IGNORE_OUTPUT, GM_COMMAND_CONTEXT.name);
                return OS_INVALID;
            }
        } else if (!strcmp(nodes[i]->element, XML_TIMEOUT)) {
            char *endptr;
            command->timeout = strtol(nodes[i]->content, &endptr, 0);

            if (*endptr || command->timeout < 0) {
                merror("Invalid content for tag '%s' at module '%s'.", XML_TIMEOUT, GM_COMMAND_CONTEXT.name);
                return OS_INVALID;
            }
        } else if (!strcmp(nodes[i]->element, XML_VERIFY_MD5)) {
            if (strlen(nodes[i]->content) != 32) {
                merror("Invalid content for tag '%s' at module '%s'.", XML_VERIFY_MD5, GM_COMMAND_CONTEXT.name);
                return OS_INVALID;
            }

            free(command->md5_hash);
            os_strdup(nodes[i]->content, command->md5_hash);
        } else if (!strcmp(nodes[i]->element, XML_VERIFY_SHA1)) {
            if (strlen(nodes[i]->content) != 40) {
                merror("Invalid content for tag '%s' at module '%s'.", XML_VERIFY_SHA1, GM_COMMAND_CONTEXT.name);
                return OS_INVALID;
            }

            free(command->sha1_hash);
            os_strdup(nodes[i]->content, command->sha1_hash);
        } else if (!strcmp(nodes[i]->element, XML_VERIFY_SHA256)) {
            if (strlen(nodes[i]->content) != 64) {
                merror("Invalid content for tag '%s' at module '%s'.", XML_VERIFY_SHA256, GM_COMMAND_CONTEXT.name);
                return OS_INVALID;
            }

            free(command->sha256_hash);
            os_strdup(nodes[i]->content, command->sha256_hash);
        } else if (!strcmp(nodes[i]->element, XML_SKIP_VERIFICATION)) {
            if (!strcmp(nodes[i]->content, "yes"))
                command->skip_verification = 1;
            else if (!strcmp(nodes[i]->content, "no"))
                command->skip_verification = 0;
            else {
                merror("Invalid content for tag '%s' at module '%s'.", XML_SKIP_VERIFICATION, GM_COMMAND_CONTEXT.name);
                return OS_INVALID;
            }
        } else if (is_sched_tag(nodes[i]->element)) {
            // Do nothing
        } else {
            merror("No such tag '%s' at module '%s'.", nodes[i]->element, GM_COMMAND_CONTEXT.name);
            return OS_INVALID;
        }
    }

    const int sched_read = sched_scan_read(&(command->scan_config), nodes, module->context->name);
    if ( sched_read != 0 ) {
        return OS_INVALID;
    }

    if (!command->tag) {
        mwarn("Option <%s> not found at module '%s'.", XML_TAG, GM_COMMAND_CONTEXT.name);
        os_strdup("", command->tag);
    }

    if (!command->command) {
        merror("Tag <%s> not found at module '%s'.", XML_COMMAND, GM_COMMAND_CONTEXT.name);
        return OS_INVALID;
    }

    return 0;
}
