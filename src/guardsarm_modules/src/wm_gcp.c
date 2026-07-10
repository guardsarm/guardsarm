/*
 * GuardSarm Module for Security Configuration Assessment
 * Copyright (C) 2026 GuardSarm, Inc.
 * January 25, 2019.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#include "wmodules.h"
#include "os_net.h"
#include <sys/stat.h>
#include "sha256_op.h"
#include "shared.h"

#ifdef GUARDSARM_UNIT_TESTING
/* Remove static qualifier when testing */
#define static
#endif

/**
 * @brief Run module function for Google Cloud Pub/Sub
 * @param data Module configuration structure
 */
static void gm_gcp_pubsub_run(const gm_gcp_pubsub *data);            // Running python script

/**
 * @brief Dump configuration structure in JSON for Google Cloud Pub/Sub
 * @param gcp_config Module configuration structure
 * @return JSON structure with module configuration
 */
cJSON *gm_gcp_pubsub_dump(const gm_gcp_pubsub *gcp_config);          // Read config
#ifdef WIN32
/**
 * @brief Main function for Google Cloud Pub/Sub
 * @param gcp_config Module configuration structure
 */
static DWORD WINAPI gm_gcp_pubsub_main(void *arg);                     // Module main function. It won't return

/**
 * @brief Main function for Google Cloud bucket
 * @param gcp_config Module configuration structure
 */
static DWORD WINAPI gm_gcp_bucket_main(void *arg);                     // Module main function. It won't return
#else
/**
 * @brief Main function for Google Cloud Pub/Sub
 * @param gcp_config Module configuration structure
 */
static void* gm_gcp_pubsub_main(gm_gcp_pubsub *gcp_config);          // Module main function. It won't return

/**
 * @brief Main function for Google Cloud bucket
 * @param gcp_config Module configuration structure
 */
static void* gm_gcp_bucket_main(gm_gcp_bucket_base *gcp_config);          // Module main function. It won't return
#endif

/**
 * @brief Free configuration structure for Google Cloud Pub/Sub
 * @param gcp_config Module configuration structure
 */
static void gm_gcp_pubsub_destroy(gm_gcp_pubsub *gcp_config);        // Destroy data

/**
 * @brief Free configuration structure for Google Cloud bucket
 * @param gcp_config Module configuration structure
 */
static void gm_gcp_bucket_destroy(gm_gcp_bucket_base *gcp_config);        // Destroy data

/**
 * @brief Run module function for Google Cloud bucket
 * @param exec_bucket Bucket configuration structure
 */
static void gm_gcp_bucket_run(gm_gcp_bucket *exec_bucket);            // Running python script

/**
 * @brief Dump configuration structure in JSON for Google Cloud bucket
 * @param gcp_config Module configuration structure
 * @return JSON structure with module configuration
 */
cJSON *gm_gcp_bucket_dump(const gm_gcp_bucket_base *gcp_config);          // Read config

/**
 * @brief Parse the output of the GCP script and prints it depending on the debug
 *        level stated by the script
 * @param output Output returned by the call to the script
 * @param tag Tag that should be used when printing the messages
 */
static void gm_gcp_parse_output(char *output, char *tag);


/* Context definition */

const gm_context GM_GCP_PUBSUB_CONTEXT = {
    .name = GCP_PUBSUB_WM_NAME,
    .start = (gm_routine)gm_gcp_pubsub_main,
    .destroy = (void(*)(void *))gm_gcp_pubsub_destroy,
    .dump = (cJSON * (*)(const void *))gm_gcp_pubsub_dump,
    .sync = NULL,
    .stop = NULL,
    .query = NULL,
};

const gm_context GM_GCP_BUCKET_CONTEXT = {
    .name = GCP_BUCKET_WM_NAME,
    .start = (gm_routine)gm_gcp_bucket_main,
    .destroy = (void(*)(void *))gm_gcp_bucket_destroy,
    .dump = (cJSON * (*)(const void *))gm_gcp_bucket_dump,
    .sync = NULL,
    .stop = NULL,
    .query = NULL,
};

#ifdef GUARDSARM_UNIT_TESTING
// Replace pthread_exit for testing purposes
#define pthread_exit(a) return a
#endif
// Module main function. It won't return
#ifdef WIN32
DWORD WINAPI gm_gcp_pubsub_main(void *arg) {
    gm_gcp_pubsub *data = (gm_gcp_pubsub *)arg;
#else
void* gm_gcp_pubsub_main(gm_gcp_pubsub *data) {
#endif
    char * timestamp = NULL;
    // If module is disabled, exit
    if (data->enabled) {
        mtinfo(GM_GCP_PUBSUB_LOGTAG, "Module started.");
    } else {
        mtinfo(GM_GCP_PUBSUB_LOGTAG, "Module disabled. Exiting.");
        pthread_exit(NULL);
    }

    do {
        const time_t time_sleep = sched_scan_get_time_until_next_scan(&(data->scan_config), GM_GCP_PUBSUB_LOGTAG, data->pull_on_start);

        if (time_sleep) {
            const int next_scan_time = sched_get_next_scan_time(data->scan_config);
            timestamp = w_get_timestamp(next_scan_time);
            mtdebug2(GM_GCP_PUBSUB_LOGTAG, "Sleeping until: %s", timestamp);
            os_free(timestamp);
            gm_sleep_until_interruptible(next_scan_time);
            if (gm_shutdown_requested) break;
        }
        if (gm_shutdown_requested) break;
        mtdebug1(GM_GCP_PUBSUB_LOGTAG, "Starting fetching of logs.");

        gm_gcp_pubsub_run(data);

        mtdebug1(GM_GCP_PUBSUB_LOGTAG, "Fetching logs finished.");
    } while (FOREVER() && !gm_shutdown_requested);

#ifdef WIN32
    return 0;
#else
    return NULL;
#endif
}

#ifdef WIN32
DWORD WINAPI gm_gcp_bucket_main(void *arg) {
    gm_gcp_bucket_base *data = (gm_gcp_bucket_base *)arg;
#else
void* gm_gcp_bucket_main(gm_gcp_bucket_base *data) {
#endif
    char * timestamp = NULL;
    // If module is disabled, exit
    if (data->enabled) {
        mtinfo(GM_GCP_BUCKET_LOGTAG, "Module started.");
    } else {
        mtinfo(GM_GCP_BUCKET_LOGTAG, "Module disabled. Exiting.");
        pthread_exit(NULL);
    }

    gm_gcp_bucket *cur_bucket;
    char *log_info;
    do {
        const time_t time_sleep = sched_scan_get_time_until_next_scan(&(data->scan_config), GM_GCP_BUCKET_LOGTAG, data->run_on_start);

        if (time_sleep) {
            const int next_scan_time = sched_get_next_scan_time(data->scan_config);
            timestamp = w_get_timestamp(next_scan_time);
            mtdebug2(GM_GCP_BUCKET_LOGTAG, "Sleeping until: %s", timestamp);
            os_free(timestamp);
            gm_sleep_until_interruptible(next_scan_time);
            if (gm_shutdown_requested) break;
        }
        if (gm_shutdown_requested) break;
        mtdebug1(GM_GCP_BUCKET_LOGTAG, "Starting fetching of logs.");

        for (cur_bucket = data->buckets; cur_bucket; cur_bucket = cur_bucket->next) {

            log_info = NULL;

            gm_strcat(&log_info, "Executing Bucket Analysis: (Bucket:", '\0');
            if (cur_bucket->bucket) {
                gm_strcat(&log_info, cur_bucket->bucket, ' ');
            }
            else {
                gm_strcat(&log_info, "unknown_bucket", ' ');
            }


            if (cur_bucket->prefix) {
                gm_strcat(&log_info, ", Path:", '\0');
                gm_strcat(&log_info, cur_bucket->prefix, ' ');
            }

            if (cur_bucket->type) {
                gm_strcat(&log_info, ", Type:", '\0');
                gm_strcat(&log_info, cur_bucket->type, ' ');
            }

            if (cur_bucket->credentials_file) {
                gm_strcat(&log_info, ", Credentials file:", '\0');
                gm_strcat(&log_info, cur_bucket->credentials_file, ' ');
            }

            gm_strcat(&log_info, ")", '\0');

            mtinfo(GM_GCP_BUCKET_LOGTAG, "%s", log_info);
            gm_gcp_bucket_run(cur_bucket);
            free(log_info);
        }

        mtdebug1(GM_GCP_BUCKET_LOGTAG, "Fetching logs finished.");
    } while (FOREVER() && !gm_shutdown_requested);

#ifdef WIN32
    return 0;
#else
    return NULL;
#endif
}

#ifdef GUARDSARM_UNIT_TESTING
/* Replace pthread_exit for testing purposes */
#undef pthread_exit
#define pthread_exit(a) return

__attribute__((weak))
#endif
void gm_gcp_pubsub_run(const gm_gcp_pubsub *data) {
    int status;
    char *output = NULL;
    char *command = NULL;

    // Create arguments
    mtdebug2(GM_GCP_PUBSUB_LOGTAG, "Create argument list");

    char * script = NULL;
    os_calloc(PATH_MAX, sizeof(char), script);

    snprintf(script, PATH_MAX, "%s", GM_GCP_SCRIPT_PATH);

    gm_strcat(&command, script, '\0');
    os_free(script);

    gm_strcat(&command, "--integration_type pubsub", ' ');

    if (data->project_id) {
        gm_strcat(&command, "--project", ' ');
        gm_strcat(&command, data->project_id, ' ');
    }
    if (data->subscription_name) {
        gm_strcat(&command, "--subscription_id", ' ');
        gm_strcat(&command, data->subscription_name, ' ');
    }
    if (data->credentials_file) {
        gm_strcat(&command, "--credentials_file", ' ');
        gm_strcat(&command, data->credentials_file, ' ');
    }

    if (data->max_messages) {
        char *int_to_string;
        os_malloc(OS_SIZE_1024, int_to_string);
        sprintf(int_to_string, "%d", data->max_messages);
        gm_strcat(&command, "--max_messages", ' ');
        gm_strcat(&command, int_to_string, ' ');
        os_free(int_to_string);
    }
    if (data->num_threads) {
        char *int_to_string;
        os_malloc(OS_SIZE_1024, int_to_string);
        sprintf(int_to_string, "%d", data->num_threads);
        gm_strcat(&command, "--num_threads", ' ');
        gm_strcat(&command, int_to_string, ' ');
        os_free(int_to_string);
    }

    if (isDebug()){
        char *int_to_string;
        os_malloc(OS_SIZE_1024, int_to_string);
        sprintf(int_to_string, "%d", isDebug());
        gm_strcat(&command, "--log_level", ' ');
        gm_strcat(&command, int_to_string, ' ');
        os_free(int_to_string);
    }

    // Execute

    mtdebug1(GM_GCP_PUBSUB_LOGTAG, "Launching command: %s", command);

    const int gm_exec_ret_code = gm_exec(command, &output, &status, 0, NULL);

    os_free(command);

    if (gm_exec_ret_code != 0){
        mterror(GM_GCP_PUBSUB_LOGTAG, "Internal error. Exiting...");
        if (gm_exec_ret_code > 0) {
            os_free(output);
        }
        pthread_exit(NULL);
    } else if (status > 0) {
        mtwarn(GM_GCP_PUBSUB_LOGTAG, "Command returned exit code %d", status);
    }

    gm_gcp_parse_output(output, GM_GCP_PUBSUB_LOGTAG);
    os_free(output);
}

static void gm_gcp_parse_output(char *output, char *tag){
    char *line;
    char * parsing_output = output;
    int debug_level = isDebug();

    for (line = strstr(parsing_output, GM_GCP_LOGGING_TOKEN); line; line = strstr(parsing_output, GM_GCP_LOGGING_TOKEN)) {
        char * tokenized_line;
        os_calloc(GM_STRING_MAX, sizeof(char), tokenized_line);
        char * next_lines;

        line += strlen(GM_GCP_LOGGING_TOKEN);
        next_lines = strstr(line, GM_GCP_LOGGING_TOKEN);

        int next_lines_chars = next_lines == NULL ? 0 : strlen(next_lines);

        // 1 is added because it's mandatory to consider the null byte
        int cp_length = 1 + strlen(line) - next_lines_chars > GM_STRING_MAX ? GM_STRING_MAX : 1 + strlen(line) - next_lines_chars;
        snprintf(tokenized_line, cp_length, "%s", line);
        if (tokenized_line[cp_length - 2] == '\n') tokenized_line[cp_length - 2] = '\0';

        char *p_line = NULL;

        if (debug_level >= 2) {
            if ((p_line = strstr(tokenized_line, "- DEBUG - "))) {
                p_line += 10;
                mtdebug1(tag, "%s", p_line);
            }
        }
        if (debug_level >= 1) {
            if ((p_line = strstr(tokenized_line, "- INFO - "))) {
                p_line += 9;
                mtinfo(tag, "%s", p_line);
            }
        }
        if (debug_level >= 0) {
            if ((p_line = strstr(tokenized_line, "- CRITICAL - "))) {
                p_line += 13;
                mterror(tag, "%s", p_line);
            }
            if ((p_line = strstr(tokenized_line, "- ERROR - "))) {
                p_line += 10;
                mterror(tag, "%s", p_line);
            }
            if ((p_line = strstr(tokenized_line, "- WARNING - "))) {
                p_line += 12;
                mtwarn(tag, "%s", p_line);
            }
        }

        parsing_output += cp_length + strlen(GM_GCP_LOGGING_TOKEN) - 1;
        os_free(tokenized_line);
    }
}

void gm_gcp_bucket_run(gm_gcp_bucket *exec_bucket) {
    int status;
    char *output = NULL;
    char *command = NULL;

    // Create arguments
    mtdebug2(GM_GCP_BUCKET_LOGTAG, "Create argument list");

    char * script = NULL;
    os_calloc(PATH_MAX, sizeof(char), script);

    snprintf(script, PATH_MAX, "%s", GM_GCP_SCRIPT_PATH);

    gm_strcat(&command, script, '\0');
    os_free(script);

    gm_strcat(&command, "--integration_type", ' ');
    gm_strcat(&command, exec_bucket->type, ' ');

    gm_strcat(&command, "--bucket_name", ' ');
    gm_strcat(&command, exec_bucket->bucket, ' ');

    if (exec_bucket->credentials_file) {
        gm_strcat(&command, "--credentials_file", ' ');
        gm_strcat(&command, exec_bucket->credentials_file, ' ');
    }
    if (exec_bucket->prefix) {
        gm_strcat(&command, "--prefix", ' ');
        gm_strcat(&command, exec_bucket->prefix, ' ');
    }
    if (exec_bucket->only_logs_after) {
        gm_strcat(&command, "--only_logs_after", ' ');
        gm_strcat(&command, exec_bucket->only_logs_after, ' ');
    }
    if (exec_bucket->remove_from_bucket) {
        gm_strcat(&command, "--remove", ' ');
    }

    if (isDebug()){
        char *int_to_string;
        os_malloc(OS_SIZE_1024, int_to_string);
        sprintf(int_to_string, "%d", isDebug());
        gm_strcat(&command, "--log_level", ' ');
        gm_strcat(&command, int_to_string, ' ');
        os_free(int_to_string);
    }

    // Execute

    mtdebug1(GM_GCP_BUCKET_LOGTAG, "Launching command: %s", command);

    const int gm_exec_ret_code = gm_exec(command, &output, &status, 0, NULL);

    os_free(command);

    if (gm_exec_ret_code != 0){
        mterror(GM_GCP_BUCKET_LOGTAG, "Internal error. Exiting...");
        if (gm_exec_ret_code > 0) {
            os_free(output);
        }
        pthread_exit(NULL);
    } else if (status > 0) {
        mtwarn(GM_GCP_BUCKET_LOGTAG, "Command returned exit code %d", status);
    }

    gm_gcp_parse_output(output, GM_GCP_BUCKET_LOGTAG);
    os_free(output);
}

void gm_gcp_pubsub_destroy(gm_gcp_pubsub * data) {
    if (data->project_id) os_free(data->project_id);
    if (data->subscription_name) os_free(data->subscription_name);
    if (data->credentials_file) os_free(data->credentials_file);
    os_free(data);
}

void gm_gcp_bucket_destroy(gm_gcp_bucket_base * data) {
    gm_gcp_bucket *cur_bucket;
    gm_gcp_bucket *next_bucket = data->buckets;
    while(next_bucket){
        cur_bucket = next_bucket;
        next_bucket = next_bucket->next;
        if (cur_bucket->bucket) os_free(cur_bucket->bucket);
        if (cur_bucket->type) os_free(cur_bucket->type);
        if (cur_bucket->credentials_file) os_free(cur_bucket->credentials_file);
        if (cur_bucket->prefix) os_free(cur_bucket->prefix);
        if (cur_bucket->only_logs_after) os_free(cur_bucket->only_logs_after);
        os_free(cur_bucket);
    }
    os_free(data);
}

cJSON *gm_gcp_pubsub_dump(const gm_gcp_pubsub *data) {
    cJSON *root = cJSON_CreateObject();
    cJSON *gm_wd = cJSON_CreateObject();

    sched_scan_dump(&(data->scan_config), gm_wd);

    cJSON_AddStringToObject(gm_wd, "enabled", data->enabled ? "yes" : "no");
    cJSON_AddStringToObject(gm_wd, "pull_on_start", data->pull_on_start ? "yes" : "no");
    if (data->max_messages) cJSON_AddNumberToObject(gm_wd, "max_messages", data->max_messages);
    if (data->num_threads) cJSON_AddNumberToObject(gm_wd, "num_threads", data->num_threads);
    if (data->project_id) cJSON_AddStringToObject(gm_wd, "project_id", data->project_id);
    if (data->subscription_name) cJSON_AddStringToObject(gm_wd, "subscription_name", data->subscription_name);
    if (data->credentials_file) cJSON_AddStringToObject(gm_wd, "credentials_file", data->credentials_file);

    int debug_level = isDebug();

    if (debug_level >= 2) cJSON_AddStringToObject(gm_wd, "logging", "debug");
    if (debug_level == 1) cJSON_AddStringToObject(gm_wd, "logging", "info");
    if (debug_level == 0) cJSON_AddStringToObject(gm_wd, "logging", "warning");

    cJSON_AddItemToObject(root, "gcp-pubsub", gm_wd);

    return root;
}

cJSON *gm_gcp_bucket_dump(const gm_gcp_bucket_base *data) {
    cJSON *root = cJSON_CreateObject();
    cJSON *gm_wd = cJSON_CreateObject();

    sched_scan_dump(&(data->scan_config), gm_wd);
    cJSON_AddStringToObject(gm_wd, "enabled", data->enabled ? "yes" : "no");
    cJSON_AddStringToObject(gm_wd, "run_on_start", data->run_on_start ? "yes" : "no");

    if (data->buckets) {
        gm_gcp_bucket *cur_bucket;
        cJSON *arr_buckets = cJSON_CreateArray();
        for (cur_bucket = data->buckets; cur_bucket; cur_bucket = cur_bucket->next) {
            cJSON *buck = cJSON_CreateObject();
            if (cur_bucket->bucket) cJSON_AddStringToObject(buck, "bucket", cur_bucket->bucket);
            if (cur_bucket->type) cJSON_AddStringToObject(buck, "type", cur_bucket->type);
            if (cur_bucket->credentials_file) cJSON_AddStringToObject(buck, "credentials_file", cur_bucket->credentials_file);
            if (cur_bucket->prefix) cJSON_AddStringToObject(buck, "prefix", cur_bucket->prefix);
            if (cur_bucket->only_logs_after) cJSON_AddStringToObject(buck, "only_logs_after", cur_bucket->only_logs_after);
            if (cur_bucket->remove_from_bucket) cJSON_AddNumberToObject(buck, "remove_from_bucket", cur_bucket->remove_from_bucket);
            cJSON_AddItemToArray(arr_buckets, buck);
        }
        if (cJSON_GetArraySize(arr_buckets) > 0) {
            cJSON_AddItemToObject(gm_wd, "buckets", arr_buckets);
        } else {
            cJSON_free(arr_buckets);
        }
    }

    int debug_level = isDebug();

    if (debug_level >= 2) cJSON_AddStringToObject(gm_wd, "logging", "debug");
    if (debug_level == 1) cJSON_AddStringToObject(gm_wd, "logging", "info");
    if (debug_level == 0) cJSON_AddStringToObject(gm_wd, "logging", "warning");

    cJSON_AddItemToObject(root, "gcp-bucket", gm_wd);

    return root;
}
