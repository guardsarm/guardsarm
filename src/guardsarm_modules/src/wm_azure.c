/*
 * GuardSarm Module for Azure integration
 * Copyright (C) 2026 GuardSarm, Inc.
 * September, 2018.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef WIN32

#include "wmodules.h"
#include "wm_azure.h"
#include "expression.h"

static gm_azure_t *azure_config;                               // Pointer to Azure-logs configuration
static w_expression_t *azure_script_log_regex;                 // Pointer to Azure script log regex
static int queue_fd;                                           // Output queue file descriptor
static unsigned int default_timeout;                           // Default timeout for every query

static void* gm_azure_main(gm_azure_t *azure_config);          // Module main function. It won't return
static void gm_azure_setup(gm_azure_t *_azure_config);         // Setup module
static void gm_azure_cleanup();                                // Cleanup function, doesn't overwrite wm_cleanup
static void gm_azure_check();                                  // Check configuration
static void gm_azure_destroy(gm_azure_t *azure_config);        // Destroy data
static void gm_setup_logging_capture();                        // Setup script logging output
static void gm_integrations_parse_output(char * const output); // Parse script logging output and log them here

static void gm_azure_log_analytics(gm_azure_api_t *log_analytics);      // Run log analytics queries
static void gm_azure_graphs(gm_azure_api_t *graph);                     // Run graph queries
static void gm_azure_storage(gm_azure_storage_t *storage);              // Run storage queries
cJSON *gm_azure_dump(const gm_azure_t *azure);                          // Dump configuration to a JSON structure

//  Azure module context definition

const gm_context GM_AZURE_CONTEXT = {
    .name = AZ_WM_NAME,
    .start = (gm_routine)gm_azure_main,
    .destroy = (void(*)(void *))gm_azure_destroy,
    .dump = (cJSON * (*)(const void *))gm_azure_dump,
    .sync = NULL,
    .stop = NULL,
    .query = NULL,
};

// Module main function. It won't return.

void* gm_azure_main(gm_azure_t *azure_config) {

    gm_azure_api_t *curr_api = NULL;
    gm_azure_storage_t *curr_storage = NULL;
    char msg[OS_SIZE_6144];
    char * timestamp = NULL;

    gm_azure_setup(azure_config);
    mtinfo(GM_AZURE_LOGTAG, "Module started.");



    // Main loop

    do {
        const time_t time_sleep = sched_scan_get_time_until_next_scan(&(azure_config->scan_config), GM_AZURE_LOGTAG, azure_config->flags.run_on_start);

        if(azure_config->state.next_time == 0) {
            azure_config->state.next_time = azure_config->scan_config.time_start + time_sleep;
        }

        if (time_sleep) {
            const int next_scan_time = sched_get_next_scan_time(azure_config->scan_config);
            timestamp = w_get_timestamp(next_scan_time);
            mtdebug2(GM_AZURE_LOGTAG, "Sleeping until: %s", timestamp);
            os_free(timestamp);
            gm_sleep_until_interruptible(next_scan_time);
            if (gm_shutdown_requested) break;
        }
        if (gm_shutdown_requested) break;
        mtinfo(GM_AZURE_LOGTAG, "Starting fetching of logs.");

        snprintf(msg, OS_SIZE_6144, "Starting Azure-logs scan.");
        SendMSG(queue_fd, msg, "rootcheck", ROOTCHECK_MQ);

        for (curr_api = azure_config->api_config; curr_api; curr_api = curr_api->next) {
            if (curr_api->type == LOG_ANALYTICS) {
                mtinfo(GM_AZURE_LOGTAG, "Starting Log Analytics collection for the domain '%s'.", curr_api->tenantdomain);
                gm_azure_log_analytics(curr_api);
                mtinfo(GM_AZURE_LOGTAG, "Finished Log Analytics collection for the domain '%s'.", curr_api->tenantdomain);
            } else if (curr_api->type == GRAPHS) {
                mtinfo(GM_AZURE_LOGTAG, "Starting Graphs log collection for the domain '%s'.", curr_api->tenantdomain);
                gm_azure_graphs(curr_api);
                mtinfo(GM_AZURE_LOGTAG, "Finished Graphs log collection for the domain '%s'.", curr_api->tenantdomain);
            }
        }

        for (curr_storage = azure_config->storage; curr_storage; curr_storage = curr_storage->next) {
            mtinfo(GM_AZURE_LOGTAG, "Starting Storage log collection for '%s'.", curr_storage->tag);
            gm_azure_storage(curr_storage);
            mtinfo(GM_AZURE_LOGTAG, "Finished Storage log collection for '%s'.", curr_storage->tag);
        }

        snprintf(msg, OS_SIZE_6144, "Ending Azure-logs scan.");
        SendMSG(queue_fd, msg, "rootcheck", ROOTCHECK_MQ);

        mtdebug1(GM_AZURE_LOGTAG, "Fetching logs finished.");

    } while (FOREVER() && !gm_shutdown_requested);

    return NULL;
}

void gm_azure_log_analytics(gm_azure_api_t *log_analytics) {

    gm_azure_request_t * curr_request = NULL;
    char query[OS_SIZE_1024];
    int status;
    unsigned int timeout;

    for (curr_request = log_analytics->request; curr_request; curr_request = curr_request->next) {

        char * command = NULL;
        char * output = NULL;

        // Create argument list
        mtdebug2(GM_AZURE_LOGTAG, "Creating argument list.");

        char * script = NULL;
        os_calloc(PATH_MAX, sizeof(char), script);
        snprintf(script, PATH_MAX, "%s", GM_AZURE_SCRIPT_PATH);
        gm_strcat(&command, script, '\0');
        os_free(script);
        gm_strcat(&command, "--log_analytics", ' ');

        if (log_analytics->auth_path) {
            gm_strcat(&command, "--la_auth_path", ' ');
            gm_strcat(&command, log_analytics->auth_path, ' ');
        } else {
            gm_strcat(&command, "--la_id", ' ');
            gm_strcat(&command, log_analytics->application_id, ' ');
            gm_strcat(&command, "--la_key", ' ');
            gm_strcat(&command, log_analytics->application_key, ' ');
        }

        gm_strcat(&command, "--la_tenant_domain", ' ');
        gm_strcat(&command, log_analytics->tenantdomain, ' ');

        gm_strcat(&command, "--la_tag", ' ');
        gm_strcat(&command, curr_request->tag, ' ');

        gm_strcat(&command, "--la_query", ' ');
        snprintf(query, OS_SIZE_1024 - 1, "\"%s\"", curr_request->query);
        gm_strcat(&command, query, ' ');

        gm_strcat(&command, "--workspace", ' ');
        gm_strcat(&command, curr_request->workspace, ' ');

        if (curr_request->time_offset) {
            gm_strcat(&command, "--la_time_offset", ' ');
            gm_strcat(&command, curr_request->time_offset, ' ');
        }
        if (isDebug()) {
            char *int_to_string;
            os_malloc(OS_SIZE_1024, int_to_string);
            sprintf(int_to_string, "%d", isDebug());
            gm_strcat(&command, "--debug", ' ');
            gm_strcat(&command, int_to_string, ' ');
            os_free(int_to_string);
        }

        // Check timeout defined
        if (curr_request->timeout)
            timeout = curr_request->timeout;
        else
            timeout = default_timeout;

        // Run script
        mtdebug1(GM_AZURE_LOGTAG, "Launching command: %s", command);
        switch (gm_exec(command, &output, &status, timeout, NULL)) {
            case 0:
                gm_integrations_parse_output(output);
                break;
            case GM_ERROR_TIMEOUT:
                mterror(GM_AZURE_LOGTAG, "Timeout expired at request '%s'.", curr_request->tag);
                break;
            default:
                mterror(GM_AZURE_LOGTAG, "Internal error. Exiting...");
                os_free(command);
                pthread_exit(NULL);
        }

        mtinfo(GM_AZURE_LOGTAG, "Finished Log Analytics collection for request '%s'.", curr_request->tag);

        os_free(command);
        os_free(output);
    }
}

void gm_azure_graphs(gm_azure_api_t *graph) {

    gm_azure_request_t * curr_request = NULL;
    char query[OS_SIZE_1024];
    int status;
    unsigned int timeout;

    for (curr_request = graph->request; curr_request; curr_request = curr_request->next) {

        char * command = NULL;
        char * output = NULL;

        // Create argument list
        mtdebug2(GM_AZURE_LOGTAG, "Creating argument list.");

        char * script = NULL;
        os_calloc(PATH_MAX, sizeof(char), script);
        snprintf(script, PATH_MAX, "%s", GM_AZURE_SCRIPT_PATH);
        gm_strcat(&command, script, '\0');
        os_free(script);
        gm_strcat(&command, "--graph", ' ');

        if (graph->auth_path) {
            gm_strcat(&command, "--graph_auth_path", ' ');
            gm_strcat(&command, graph->auth_path, ' ');
        } else {
            gm_strcat(&command, "--graph_id", ' ');
            gm_strcat(&command, graph->application_id, ' ');
            gm_strcat(&command, "--graph_key", ' ');
            gm_strcat(&command, graph->application_key, ' ');
        }

        gm_strcat(&command, "--graph_tenant_domain", ' ');
        gm_strcat(&command, graph->tenantdomain, ' ');

        gm_strcat(&command, "--graph_tag", ' ');
        gm_strcat(&command, curr_request->tag, ' ');

        gm_strcat(&command, "--graph_query", ' ');
        snprintf(query, OS_SIZE_1024 - 1, "\'%s\'", curr_request->query);
        gm_strcat(&command, query, ' ');

        if (curr_request->time_offset) {
            gm_strcat(&command, "--graph_time_offset", ' ');
            gm_strcat(&command, curr_request->time_offset, ' ');
        }

        if (isDebug()) {
            char *int_to_string;
            os_malloc(OS_SIZE_1024, int_to_string);
            sprintf(int_to_string, "%d", isDebug());
            gm_strcat(&command, "--debug", ' ');
            gm_strcat(&command, int_to_string, ' ');
            os_free(int_to_string);
        }

        // Check timeout defined
        if (curr_request->timeout)
            timeout = curr_request->timeout;
        else
            timeout = default_timeout;

        // Run script
        mtdebug1(GM_AZURE_LOGTAG, "Launching command: %s", command);
        switch (gm_exec(command, &output, &status, timeout, NULL)) {
            case 0:
                gm_integrations_parse_output(output);
                break;
            case GM_ERROR_TIMEOUT:
                mterror(GM_AZURE_LOGTAG, "Timeout expired at request '%s'.", curr_request->tag);
                break;
            default:
                mterror(GM_AZURE_LOGTAG, "Internal error. Exiting...");
                os_free(command);
                pthread_exit(NULL);
        }

        mtinfo(GM_AZURE_LOGTAG, "Finished Graphs log collection for request '%s'.", curr_request->tag);

        os_free(command);
        os_free(output);
    }
}

void gm_azure_storage(gm_azure_storage_t *storage) {

    gm_azure_container_t * curr_container = NULL;
    char name[OS_SIZE_256];
    char blobs[OS_SIZE_256];
    int status;
    unsigned int timeout;

    for (curr_container = storage->container; curr_container; curr_container = curr_container->next) {

        char * command = NULL;
        char * output = NULL;

        // Create argument list
        mtdebug2(GM_AZURE_LOGTAG, "Creating argument list.");

        char * script = NULL;
        os_calloc(PATH_MAX, sizeof(char), script);
        snprintf(script, PATH_MAX, "%s", GM_AZURE_SCRIPT_PATH);
        gm_strcat(&command, script, '\0');
        os_free(script);
        gm_strcat(&command, "--storage", ' ');

        if (storage->auth_path) {
            gm_strcat(&command, "--storage_auth_path", ' ');
            gm_strcat(&command, storage->auth_path, ' ');
        } else {
            gm_strcat(&command, "--account_name", ' ');
            gm_strcat(&command, storage->account_name, ' ');
            gm_strcat(&command, "--account_key", ' ');
            gm_strcat(&command, storage->account_key, ' ');
        }

        gm_strcat(&command, "--container", ' ');
        snprintf(name, OS_SIZE_256 - 1, "\"%s\"", curr_container->name);
        gm_strcat(&command, name, ' ');

        gm_strcat(&command, "--blobs", ' ');
        if (curr_container->blobs)
            snprintf(blobs, OS_SIZE_256 - 1, "\"%s\"", curr_container->blobs);
        else
            snprintf(blobs, OS_SIZE_256 -1, "\"*\"");
        gm_strcat(&command, blobs, ' ');

        gm_strcat(&command, "--storage_tag", ' ');
        gm_strcat(&command, storage->tag, ' ');

        if (curr_container->content_type) {
            if (!strncmp(curr_container->content_type, "json_file", 9)) {
                gm_strcat(&command, "--json_file", ' ');
            } else if (!strncmp(curr_container->content_type, "json_inline", 11)) {
                gm_strcat(&command, "--json_inline", ' ');
            }
        }

        if (curr_container->time_offset) {
            gm_strcat(&command, "--storage_time_offset", ' ');
            gm_strcat(&command, curr_container->time_offset, ' ');
        }

        if (curr_container->path) {
            gm_strcat(&command, "--prefix", ' ');
            gm_strcat(&command, curr_container->path, ' ');
        }

        if (isDebug()) {
            char *int_to_string;
            os_malloc(OS_SIZE_1024, int_to_string);
            sprintf(int_to_string, "%d", isDebug());
            gm_strcat(&command, "--debug", ' ');
            gm_strcat(&command, int_to_string, ' ');
            os_free(int_to_string);
        }

        // Check timeout defined
        if (curr_container->timeout)
            timeout = curr_container->timeout;
        else
            timeout = default_timeout;

        // Run script
        mtdebug1(GM_AZURE_LOGTAG, "Launching command: %s", command);
        switch (gm_exec(command, &output, &status, timeout, NULL)) {
            case 0:
                gm_integrations_parse_output(output);
                break;
            case GM_ERROR_TIMEOUT:
                mterror(GM_AZURE_LOGTAG, "Timeout expired at request '%s'.", curr_container->name);
                break;
            default:
                mterror(GM_AZURE_LOGTAG, "Internal error. Exiting...");
                os_free(command);
                pthread_exit(NULL);
        }

        mtinfo(GM_AZURE_LOGTAG, "Finished Storage log collection for container '%s'.", curr_container->name);

        os_free(command);
        os_free(output);
    }
}

// Setup module

void gm_azure_setup(gm_azure_t *_azure_config) {

    azure_config = _azure_config;
    gm_azure_check();
    gm_setup_logging_capture();

    // Read running state

    if (gm_state_io(GM_AZURE_CONTEXT.name, GM_IO_READ, &azure_config->state, sizeof(azure_config->state)) < 0)
        memset(&azure_config->state, 0, sizeof(azure_config->state));

    // Connect to socket

    queue_fd = StartMQ(DEFAULTQUEUE, WRITE, INFINITE_OPENQ_ATTEMPTS);

    if (queue_fd < 0) {
        mterror(GM_AZURE_LOGTAG, "Can't connect to queue.");
        pthread_exit(NULL);
    }

    // Cleanup exiting

    atexit(gm_azure_cleanup);
}

// Check configuration

void gm_azure_check() {

    // Check if disabled
    if (!azure_config->flags.enabled) {
        mtinfo(GM_AZURE_LOGTAG, "Module disabled. Exiting...");
        pthread_exit(NULL);
    }

    // Check if necessary configuration is defined

    if (!azure_config->api_config && !azure_config->storage) {
        mtwarn(GM_AZURE_LOGTAG, "No API (log_analytics, graph or storage) defined. Exiting...");
        pthread_exit(NULL);
    }

    // Set default timeout
    default_timeout = azure_config->timeout;

}

// Cleanup function, doesn't overwrite wm_cleanup

void gm_azure_cleanup() {
    close(queue_fd);
    mtinfo(GM_AZURE_LOGTAG, "Module finished.");
}

// Destroy data

void gm_azure_destroy(gm_azure_t *azure_config) {

    gm_azure_api_t *curr_api = NULL;
    gm_azure_api_t *next_api = NULL;
    gm_azure_storage_t *curr_storage = NULL;
    gm_azure_storage_t *next_storage = NULL;

    for (curr_api = azure_config->api_config; curr_api; curr_api = next_api) {

        next_api = curr_api->next;
        free(curr_api->tenantdomain);
        if (curr_api->application_id)
            free(curr_api->application_id);
        if (curr_api->application_key)
            free(curr_api->application_key);
        if (curr_api->auth_path)
            free(curr_api->auth_path);

        gm_azure_request_t *curr_request = NULL;
        gm_azure_request_t *next_request = NULL;

        for (curr_request = curr_api->request; curr_request; curr_request = next_request) {

            next_request = curr_request->next;
            free(curr_request->tag);
            free(curr_request->query);
            free(curr_request->time_offset);
            if (curr_api->type == LOG_ANALYTICS)
                free(curr_request->workspace);

            free(curr_request);
        }

        free(curr_api);
    }

    for (curr_storage = azure_config->storage; curr_storage; curr_storage = next_storage) {

        next_storage = curr_storage->next;
        free(curr_storage->tag);
        if (curr_storage->account_name)
            free(curr_storage->account_name);
        if (curr_storage->account_key)
            free(curr_storage->account_key);
        if (curr_storage->auth_path)
            free(curr_storage->auth_path);

        gm_azure_container_t *curr_container = NULL;
        gm_azure_container_t *next_container = NULL;

        for (curr_container = curr_storage->container; curr_container; curr_container = next_container) {

            next_container = curr_container->next;
            free(curr_container->name);
            free(curr_container->blobs);
            free(curr_container->content_type);
            free(curr_container->time_offset);
            free(curr_container->path);
            free(curr_container);

        }

        free(curr_storage);
    }

    free(azure_config);
    w_free_expression(azure_script_log_regex);
}


// Get configuration data in JSON format

cJSON *gm_azure_dump(const gm_azure_t * azure) {

    cJSON *root = cJSON_CreateObject();
    cJSON *gm_azure = cJSON_CreateObject();

    if (azure->flags.enabled) cJSON_AddStringToObject(gm_azure,"disabled","no"); else cJSON_AddStringToObject(gm_azure,"disabled","yes");
    if (azure->flags.run_on_start) cJSON_AddStringToObject(gm_azure,"run_on_start","yes"); else cJSON_AddStringToObject(gm_azure,"run_on_start","no");
    sched_scan_dump(&(azure->scan_config), gm_azure);
    cJSON_AddNumberToObject(gm_azure,"timeout",azure->timeout);

    if (azure->api_config || azure->storage) {
        cJSON *content = cJSON_CreateArray();
        gm_azure_api_t * api_config;
        gm_azure_storage_t * storage_conf;
        for (api_config = azure->api_config; api_config; api_config = api_config->next) {
            cJSON *api = cJSON_CreateObject();
            if (api_config->type == LOG_ANALYTICS) cJSON_AddStringToObject(api, "type", "log_analytics"); else cJSON_AddStringToObject(api, "type", "graph");
            cJSON_AddStringToObject(api, "tenantdomain", api_config->tenantdomain);
            if (api_config->application_id) cJSON_AddStringToObject(api, "application_id", api_config->application_id);
            if (api_config->application_key) cJSON_AddStringToObject(api, "application_key", api_config->application_key);
            if (api_config->auth_path) cJSON_AddStringToObject(api, "auth_path", api_config->auth_path);
            if (api_config->request) {
                cJSON *requests = cJSON_CreateArray();
                gm_azure_request_t * request_conf;
                for (request_conf = api_config->request; request_conf; request_conf = request_conf->next) {
                    cJSON * request = cJSON_CreateObject();
                    cJSON_AddStringToObject(request, "tag", request_conf->tag);
                    cJSON_AddStringToObject(request, "query", request_conf->query);
                    cJSON_AddStringToObject(request, "time_offset", request_conf->time_offset);
                    if (request_conf->workspace) cJSON_AddStringToObject(request, "workspace", request_conf->workspace);
                    if (request_conf->timeout) cJSON_AddNumberToObject(request, "timeout", request_conf->timeout);
                    cJSON_AddItemToArray(requests, request);
                }
            }
            cJSON_AddItemToArray(content, api);
        }
        for (storage_conf = azure->storage; storage_conf; storage_conf = storage_conf->next) {
            cJSON *storage = cJSON_CreateObject();
            cJSON_AddStringToObject(storage, "tag", storage_conf->tag);
            if (storage_conf->account_name) cJSON_AddStringToObject(storage, "account_name", storage_conf->account_name);
            if (storage_conf->account_key) cJSON_AddStringToObject(storage, "account_key", storage_conf->account_key);
            if (storage_conf->auth_path) cJSON_AddStringToObject(storage, "auth_path", storage_conf->auth_path);
            if (storage_conf->container) {
                cJSON *containers = cJSON_CreateArray();
                gm_azure_container_t * container_conf;
                for (container_conf = storage_conf->container; container_conf; container_conf = container_conf->next) {
                    cJSON * container = cJSON_CreateObject();
                    cJSON_AddStringToObject(container, "name", container_conf->name);
                    cJSON_AddStringToObject(container, "blobs", container_conf->blobs);
                    cJSON_AddStringToObject(container, "content_type", container_conf->content_type);
                    cJSON_AddStringToObject(container, "time_offset", container_conf->time_offset);
                    cJSON_AddStringToObject(container, "prefix", container_conf->path);
                    if (container_conf->timeout) cJSON_AddNumberToObject(container, "timeout", container_conf->timeout);
                    cJSON_AddItemToArray(containers, container);
                }
            }
            cJSON_AddItemToArray(content, storage);
        }
        cJSON_AddItemToObject(gm_azure, "content", content);
    }

    cJSON_AddItemToObject(root, "azure-logs", gm_azure);

    return root;
}

// Setup script logging capture feature

static void gm_setup_logging_capture() {
    const char * const log_pattern =
        "^\\d{4}/\\d{2}/\\d{2} \\d{2}:\\d{2}:\\d{2} azure: (DEBUG|INFO|WARNING|ERROR): ";
    w_calloc_expression_t(&azure_script_log_regex, EXP_TYPE_PCRE2);

    if (!w_expression_compile(azure_script_log_regex, log_pattern, 0)) {
        mterror(GM_AZURE_LOGTAG, REGEX_COMPILE_2, log_pattern);
        pthread_exit(NULL);
    }
}

// Get script logging output and log it here

static void gm_integrations_parse_output(char * const output) {
    char *saveptr = NULL;
    const char *end_match = NULL;
    char *log_line = strtok_r(output, "\n", &saveptr);
    regex_matching *regex_match = NULL;

    while (log_line != NULL) {
        os_calloc(1, sizeof(regex_matching), regex_match);
        if (w_expression_match(azure_script_log_regex, log_line, &end_match, regex_match) &&
            regex_match->sub_strings && regex_match->sub_strings[0] && end_match) {

            char * const log_level = regex_match->sub_strings[0];
            char * const log_payload = (char *)(end_match + 1);

            if (!strcmp(log_level, "INFO")) {
                mtinfo(GM_AZURE_LOGTAG, "%s", log_payload);
            } else if (!strcmp(log_level, "WARNING")) {
                mtwarn(GM_AZURE_LOGTAG, "%s", log_payload);
            } else if (!strcmp(log_level, "ERROR")) {
                mterror(GM_AZURE_LOGTAG, "%s", log_payload);
            } else if (!strcmp(log_level, "DEBUG")) {
                mtdebug1(GM_AZURE_LOGTAG, "%s", log_payload);
            }
        }

        w_free_expression_match(azure_script_log_regex, &regex_match);
        log_line = strtok_r(NULL, "\n", &saveptr);
    }
}

#endif
