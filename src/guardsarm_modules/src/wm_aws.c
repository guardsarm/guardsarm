/*
 * GuardSarm Module for AWS S3 integration
 * Copyright (C) 2026 GuardSarm, Inc.
 * January 08, 2018.
 *
 * Updated by Jeremy Phillips <jeremy@uranusbytes.com>
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#include "wmodules.h"

#ifdef GUARDSARM_UNIT_TESTING
/* Remove static qualifier when testing */
#define static
#endif

static gm_aws *aws_config;                              // Pointer to aws_configuration
#ifdef WIN32
static DWORD WINAPI gm_aws_main(void *arg);             // Module main function. It won't return
#else
static void* gm_aws_main(gm_aws *aws_config);           // Module main function. It won't return
#endif
static void gm_aws_destroy(gm_aws *aws_config);         // Destroy data
static void gm_aws_setup(gm_aws *_aws_config);          // Setup module
static void gm_aws_check();                             // Check configuration, disable flag
static void gm_aws_run_s3(gm_aws *aws_config, gm_aws_bucket *bucket);       // Run a s3 bucket
static void gm_aws_run_service(gm_aws *aws_config, gm_aws_service *service);// Run a AWS service such as Inspector
static void gm_aws_run_subscriber(gm_aws *aws_config, gm_aws_subscriber *subscriber); //Run an AWS subscriber
cJSON *gm_aws_dump(const gm_aws *aws_config);

// Command module context definition

const gm_context GM_AWS_CONTEXT = {
    .name = "aws-s3",
    .start = (gm_routine)gm_aws_main,
    .destroy = (void(*)(void *))gm_aws_destroy,
    .dump = (cJSON * (*)(const void *))gm_aws_dump,
    .sync = NULL,
    .stop = NULL,
    .query = NULL,
};

// Module module main function. It won't return.
#ifdef WIN32
DWORD WINAPI gm_aws_main(void *arg) {
    gm_aws *aws_config = (gm_aws *)arg;
#else
void* gm_aws_main(gm_aws *aws_config) {
#endif
    gm_aws_bucket *cur_bucket;
    gm_aws_service *cur_service;
    gm_aws_subscriber *cur_subscriber;
    char *log_info;
    char * timestamp = NULL;


    gm_aws_setup(aws_config);
    mtinfo(GM_AWS_LOGTAG, "Module AWS started");

    // Main loop

    do {

        const time_t time_sleep = sched_scan_get_time_until_next_scan(&(aws_config->scan_config), GM_AWS_LOGTAG, aws_config->run_on_start);

        if (aws_config->state.next_time == 0) {
            aws_config->state.next_time = aws_config->scan_config.time_start + time_sleep;
        }

        if (gm_state_io(GM_AWS_CONTEXT.name, GM_IO_WRITE, &aws_config->state, sizeof(aws_config->state)) < 0)
            mterror(GM_AWS_LOGTAG, "Couldn't save running state.");

        if (time_sleep) {
            const int next_scan_time = sched_get_next_scan_time(aws_config->scan_config);
            timestamp = w_get_timestamp(next_scan_time);
            mtdebug2(GM_AWS_LOGTAG, "Sleeping until: %s", timestamp);
            os_free(timestamp);
            gm_sleep_until_interruptible(next_scan_time);
            if (gm_shutdown_requested) break;
        }
        if (gm_shutdown_requested) break;
        mtinfo(GM_AWS_LOGTAG, "Starting fetching of logs.");

        for (cur_bucket = aws_config->buckets; cur_bucket; cur_bucket = cur_bucket->next) {

            log_info = NULL;

            gm_strcat(&log_info, "Executing Bucket Analysis: (Bucket:", '\0');
            if (cur_bucket->bucket) {
                gm_strcat(&log_info, cur_bucket->bucket, ' ');
            }
            else {
                gm_strcat(&log_info, "unknown_bucket", ' ');
            }


            if (cur_bucket->trail_prefix) {
                gm_strcat(&log_info, ", Path:", '\0');
                gm_strcat(&log_info, cur_bucket->trail_prefix, ' ');
            }

            if (cur_bucket->trail_suffix) {
                gm_strcat(&log_info, ", Path suffix:", '\0');
                gm_strcat(&log_info, cur_bucket->trail_suffix, ' ');
            }

            if (cur_bucket->type) {
                gm_strcat(&log_info, ", Type:", '\0');
                gm_strcat(&log_info, cur_bucket->type, ' ');
            }

            if (cur_bucket->aws_account_id) {
                gm_strcat(&log_info, ", Account ID:", '\0');
                gm_strcat(&log_info, cur_bucket->aws_account_id, ' ');
            }

            if (cur_bucket->aws_account_alias) {
                gm_strcat(&log_info, ", Account Alias:", '\0');
                gm_strcat(&log_info, cur_bucket->aws_account_alias, ' ');
            }

            if (cur_bucket->aws_organization_id) {
                gm_strcat(&log_info, ", Organization ID:", '\0');
                gm_strcat(&log_info, cur_bucket->aws_organization_id, ' ');
            }

            if (cur_bucket->aws_profile) {
                gm_strcat(&log_info, ", Profile:", '\0');
                gm_strcat(&log_info, cur_bucket->aws_profile, ' ');
            }

            gm_strcat(&log_info, ")", '\0');

            mtinfo(GM_AWS_LOGTAG, "%s", log_info);
            gm_aws_run_s3(aws_config, cur_bucket);
            free(log_info);
        }

        for (cur_service = aws_config->services; cur_service; cur_service = cur_service->next) {

            log_info = NULL;

            gm_strcat(&log_info, "Executing Service Analysis: (Service:", '\0');
            if (cur_service->type) {
                gm_strcat(&log_info, cur_service->type, ' ');
            }
            else {
                gm_strcat(&log_info, "unknown_type", ' ');
            }


            if (cur_service->aws_account_id) {
                gm_strcat(&log_info, ", Account ID:", '\0');
                gm_strcat(&log_info, cur_service->aws_account_id, ' ');
            }

            if (cur_service->aws_account_alias) {
                gm_strcat(&log_info, ", Account Alias:", '\0');
                gm_strcat(&log_info, cur_service->aws_account_alias, ' ');
            }

            if (cur_service->aws_profile) {
                gm_strcat(&log_info, ", Profile:", '\0');
                gm_strcat(&log_info, cur_service->aws_profile, ' ');
            }

            gm_strcat(&log_info, ")", '\0');

            mtinfo(GM_AWS_LOGTAG, "%s", log_info);
            gm_aws_run_service(aws_config, cur_service);
            free(log_info);
        }

        for (cur_subscriber = aws_config->subscribers; cur_subscriber; cur_subscriber = cur_subscriber->next) {
            log_info = NULL;

            gm_strcat(&log_info, "Executing Subscriber fetch: (Type and SQS:", '\0');
            if (cur_subscriber->type) {
                gm_strcat(&log_info, cur_subscriber->type, ' ');
            }
            else {
                gm_strcat(&log_info, "unknown_type", ' ');
            }

            if (cur_subscriber->sqs_name) {
                gm_strcat(&log_info, cur_subscriber->sqs_name, ' ');
            }
            else {
                gm_strcat(&log_info, "unknown_queue", ' ');
            }

            gm_strcat(&log_info, ")", '\0');

            mtinfo(GM_AWS_LOGTAG, "%s", log_info);
            gm_aws_run_subscriber(aws_config, cur_subscriber);
            free(log_info);
        }

        mtinfo(GM_AWS_LOGTAG, "Fetching logs finished.");

    } while (FOREVER() && !gm_shutdown_requested);

#ifdef WIN32
    return 0;
#else
    return NULL;
#endif
}


// Get read data

cJSON *gm_aws_dump(const gm_aws *aws_config) {

    cJSON *root = cJSON_CreateObject();
    cJSON *gm_aws = cJSON_CreateObject();

    sched_scan_dump(&(aws_config->scan_config), gm_aws);

    if (aws_config->enabled) cJSON_AddStringToObject(gm_aws,"disabled","no"); else cJSON_AddStringToObject(gm_aws,"disabled","yes");
    if (aws_config->run_on_start) cJSON_AddStringToObject(gm_aws,"run_on_start","yes"); else cJSON_AddStringToObject(gm_aws,"run_on_start","no");
    if (aws_config->skip_on_error) cJSON_AddStringToObject(gm_aws,"skip_on_error","yes"); else cJSON_AddStringToObject(gm_aws,"skip_on_error","no");
    if (aws_config->buckets) {
        gm_aws_bucket *iter;
        cJSON *arr_buckets = cJSON_CreateArray();
        for (iter = aws_config->buckets; iter; iter = iter->next) {
            cJSON *buck = cJSON_CreateObject();
            if (iter->bucket) cJSON_AddStringToObject(buck,"name",iter->bucket);
            if (iter->access_key) cJSON_AddStringToObject(buck,"access_key",iter->access_key);
            if (iter->secret_key) cJSON_AddStringToObject(buck,"secret_key",iter->secret_key);
            if (iter->aws_profile) cJSON_AddStringToObject(buck,"aws_profile",iter->aws_profile);
            if (iter->iam_role_arn) cJSON_AddStringToObject(buck,"iam_role_arn",iter->iam_role_arn);
            if (iter->iam_role_duration) cJSON_AddStringToObject(buck, "iam_role_duration",iter->iam_role_duration);
            if (iter->aws_account_id) cJSON_AddStringToObject(buck,"aws_account_id",iter->aws_account_id);
            if (iter->aws_account_alias) cJSON_AddStringToObject(buck,"aws_account_alias",iter->aws_account_alias);
            if (iter->trail_prefix) cJSON_AddStringToObject(buck,"path",iter->trail_prefix);
            if (iter->trail_suffix) cJSON_AddStringToObject(buck,"path_suffix",iter->trail_suffix);
            if (iter->only_logs_after) cJSON_AddStringToObject(buck,"only_logs_after",iter->only_logs_after);
            if (iter->regions) cJSON_AddStringToObject(buck,"regions",iter->regions);
            if (iter->type) cJSON_AddStringToObject(buck,"type",iter->type);
            if (iter->remove_from_bucket) cJSON_AddStringToObject(buck,"remove_from_bucket","yes"); else cJSON_AddStringToObject(buck,"remove_from_bucket","no");
            if (iter->discard_field) cJSON_AddStringToObject(buck,"discard_field",iter->discard_field);
            if (iter->discard_regex) cJSON_AddStringToObject(buck,"discard_regex",iter->discard_regex);
            if (iter->sts_endpoint) cJSON_AddStringToObject(buck,"sts_endpoint",iter->sts_endpoint);
            if (iter->service_endpoint) cJSON_AddStringToObject(buck,"service_endpoint",iter->service_endpoint);
            cJSON_AddItemToArray(arr_buckets,buck);
        }
        if (cJSON_GetArraySize(arr_buckets) > 0) {
            cJSON_AddItemToObject(gm_aws,"buckets",arr_buckets);
        } else {
            cJSON_free(arr_buckets);
        }
    }
    if (aws_config->services) {
        gm_aws_service *iter;
        cJSON *arr_services = cJSON_CreateArray();
        for (iter = aws_config->services; iter; iter = iter->next) {
            cJSON *service = cJSON_CreateObject();
            if (iter->type) cJSON_AddStringToObject(service,"type",iter->type); // type is the name of the service
            if (iter->access_key) cJSON_AddStringToObject(service,"access_key",iter->access_key);
            if (iter->secret_key) cJSON_AddStringToObject(service,"secret_key",iter->secret_key);
            if (iter->aws_profile) cJSON_AddStringToObject(service,"aws_profile",iter->aws_profile);
            if (iter->iam_role_arn) cJSON_AddStringToObject(service,"iam_role_arn",iter->iam_role_arn);
            if (iter->iam_role_duration) cJSON_AddStringToObject(service, "iam_role_duration",iter->iam_role_duration);
            if (iter->aws_account_id) cJSON_AddStringToObject(service,"aws_account_id",iter->aws_account_id);
            if (iter->aws_account_alias) cJSON_AddStringToObject(service,"aws_account_alias",iter->aws_account_alias);
            if (iter->only_logs_after) cJSON_AddStringToObject(service,"only_logs_after",iter->only_logs_after);
            if (iter->regions) cJSON_AddStringToObject(service,"regions",iter->regions);
            if (iter->aws_log_groups) cJSON_AddStringToObject(service,"aws_log_groups",iter->aws_log_groups);
            if (iter->remove_log_streams) cJSON_AddStringToObject(service,"remove_log_streams","yes"); else cJSON_AddStringToObject(service,"remove_log_streams","no");
            if (iter->discard_field) cJSON_AddStringToObject(service,"discard_field",iter->discard_field);
            if (iter->discard_regex) cJSON_AddStringToObject(service,"discard_regex",iter->discard_regex);
            if (iter->sts_endpoint) cJSON_AddStringToObject(service,"sts_endpoint",iter->sts_endpoint);
            if (iter->service_endpoint) cJSON_AddStringToObject(service,"service_endpoint",iter->service_endpoint);
            cJSON_AddItemToArray(arr_services,service);
        }
        if (cJSON_GetArraySize(arr_services) > 0) {
            cJSON_AddItemToObject(gm_aws,"services",arr_services);
        } else {
            cJSON_free(arr_services);
        }
    }
    if (aws_config->subscribers) {
    gm_aws_subscriber *iter;
        cJSON *arr_subscribers = cJSON_CreateArray();
        for (iter = aws_config->subscribers; iter; iter = iter->next) {
            cJSON *subscriber = cJSON_CreateObject();
            if (iter->type) cJSON_AddStringToObject(subscriber,"type",iter->type);
            if (iter->sqs_name) cJSON_AddStringToObject(subscriber,"sqs_name",iter->sqs_name);
            if (iter->external_id) cJSON_AddStringToObject(subscriber,"external_id",iter->external_id);
            if (iter->iam_role_arn) cJSON_AddStringToObject(subscriber,"iam_role_arn",iter->iam_role_arn);
            if (iter->iam_role_duration) cJSON_AddStringToObject(subscriber, "iam_role_duration",iter->iam_role_duration);
            if (iter->aws_profile) cJSON_AddStringToObject(subscriber,"aws_profile",iter->aws_profile);
            if (iter->sts_endpoint) cJSON_AddStringToObject(subscriber,"sts_endpoint",iter->sts_endpoint);
            if (iter->service_endpoint) cJSON_AddStringToObject(subscriber,"service_endpoint",iter->service_endpoint);
            if (iter->discard_field) cJSON_AddStringToObject(subscriber,"discard_field",iter->discard_field);
            if (iter->discard_regex) cJSON_AddStringToObject(subscriber,"discard_regex",iter->discard_regex);
            cJSON_AddItemToArray(arr_subscribers,subscriber);
        }
        if (cJSON_GetArraySize(arr_subscribers) > 0) {
            cJSON_AddItemToObject(gm_aws,"subscribers",arr_subscribers);
        } else {
            cJSON_free(arr_subscribers);
        }
    }
    cJSON_AddItemToObject(root,"aws-s3",gm_aws);

    return root;
}


// Destroy data
void gm_aws_destroy(gm_aws *aws_config) {
    free(aws_config);
}

// Setup module

void gm_aws_setup(gm_aws *_aws_config) {

    aws_config = _aws_config;
    gm_aws_check();

    // Read running state

    if (gm_state_io(GM_AWS_CONTEXT.name, GM_IO_READ, &aws_config->state, sizeof(aws_config->state)) < 0)
        memset(&aws_config->state, 0, sizeof(aws_config->state));

    // Connect to socket

    aws_config->queue_fd = StartMQ(DEFAULTQUEUE, WRITE, INFINITE_OPENQ_ATTEMPTS);

    if (aws_config->queue_fd < 0) {
        mterror(GM_AWS_LOGTAG, "Can't connect to queue.");
        pthread_exit(NULL);
    }
}


// Check configuration

void gm_aws_check() {
    // Check if disabled

    if (!aws_config->enabled) {
        mtinfo(GM_AWS_LOGTAG, "Module AWS is disabled. Exiting...");
        pthread_exit(NULL);
    }

    // Check if there are buckets or services

    if (!aws_config->buckets && !aws_config->services && !aws_config->subscribers) {
        mtwarn(GM_AWS_LOGTAG, "No AWS buckets, services or subscribers defined. Exiting...");
        pthread_exit(NULL);
    }

    // Check if interval defined; otherwise set default

    if (!aws_config->scan_config.interval)
        aws_config->scan_config.interval = GM_AWS_DEFAULT_INTERVAL;

}

// Run a bucket parsing
#ifdef GUARDSARM_UNIT_TESTING
__attribute__((weak))
#endif
void gm_aws_run_s3(gm_aws *aws_config, gm_aws_bucket *exec_bucket) {
    int status;
    char *output = NULL;
    char *command = NULL;

    // Define time to sleep between messages sent
    int usec = 1000000 / gm_max_eps;

    // Create arguments
    mtdebug2(GM_AWS_LOGTAG, "Create argument list");

    // script path
    char * script = NULL;
    os_calloc(PATH_MAX, sizeof(char), script);

    snprintf(script, PATH_MAX, "%s", GM_AWS_SCRIPT_PATH);

    gm_strcat(&command, script, '\0');
    os_free(script);

    // bucket
    gm_strcat(&command, "--bucket", ' ');
    gm_strcat(&command, exec_bucket->bucket, ' ');

    // bucket arguments
    if (exec_bucket->remove_from_bucket) {
        gm_strcat(&command, "--remove", ' ');
    }
    if (exec_bucket->access_key) {
        gm_strcat(&command, "--access_key", ' ');
        gm_strcat(&command, exec_bucket->access_key, ' ');
    }
    if (exec_bucket->secret_key) {
        gm_strcat(&command, "--secret_key", ' ');
        gm_strcat(&command, exec_bucket->secret_key, ' ');
    }
    if (exec_bucket->aws_profile) {
        gm_strcat(&command, "--aws_profile", ' ');
        gm_strcat(&command, exec_bucket->aws_profile, ' ');
    }
    if (exec_bucket->iam_role_arn) {
        gm_strcat(&command, "--iam_role_arn", ' ');
        gm_strcat(&command, exec_bucket->iam_role_arn, ' ');
    }
    if (exec_bucket->iam_role_duration){
        gm_strcat(&command, "--iam_role_duration", ' ');
        gm_strcat(&command, exec_bucket->iam_role_duration, ' ');
    }
    if (exec_bucket->aws_organization_id) {
        gm_strcat(&command, "--aws_organization_id", ' ');
        gm_strcat(&command, exec_bucket->aws_organization_id, ' ');
    }
    if (exec_bucket->aws_account_id) {
        gm_strcat(&command, "--aws_account_id", ' ');
        gm_strcat(&command, exec_bucket->aws_account_id, ' ');
    }
    if (exec_bucket->aws_account_alias) {
        gm_strcat(&command, "--aws_account_alias", ' ');
        gm_strcat(&command, exec_bucket->aws_account_alias, ' ');
    }
    if (exec_bucket->trail_prefix) {
        gm_strcat(&command, "--trail_prefix", ' ');
        gm_strcat(&command, exec_bucket->trail_prefix, ' ');
    }
    if (exec_bucket->trail_suffix) {
        gm_strcat(&command, "--trail_suffix", ' ');
        gm_strcat(&command, exec_bucket->trail_suffix, ' ');
    }
    if (exec_bucket->only_logs_after) {
        gm_strcat(&command, "--only_logs_after", ' ');
        gm_strcat(&command, exec_bucket->only_logs_after, ' ');
    }
    if (exec_bucket->regions) {
        gm_strcat(&command, "--regions", ' ');
        gm_strcat(&command, exec_bucket->regions, ' ');
    }
    if (exec_bucket->discard_field) {
        gm_strcat(&command, "--discard-field", ' ');
        gm_strcat(&command, exec_bucket->discard_field, ' ');
    }
    if (exec_bucket->discard_regex) {
        gm_strcat(&command, "--discard-regex", ' ');
        gm_strcat(&command, exec_bucket->discard_regex, ' ');
    }
    if (exec_bucket->sts_endpoint) {
        gm_strcat(&command, "--sts_endpoint", ' ');
        gm_strcat(&command, exec_bucket->sts_endpoint, ' ');
    }
    if (exec_bucket->service_endpoint) {
        gm_strcat(&command, "--service_endpoint", ' ');
        gm_strcat(&command, exec_bucket->service_endpoint, ' ');
    }
    if (exec_bucket->type) {
        gm_strcat(&command, "--type", ' ');
        gm_strcat(&command, exec_bucket->type, ' ');
    }
    if (isDebug()) {
        gm_strcat(&command, "--debug", ' ');
        if (isDebug() > 2) {
            gm_strcat(&command, "3", ' ');
        } else if (isDebug() > 1) {
            gm_strcat(&command, "2", ' ');
        } else {
            gm_strcat(&command, "1", ' ');
        }
    }
    if (aws_config->skip_on_error) {
        gm_strcat(&command, "--skip_on_error", ' ');
    }
    if (gm_state_io(GM_AWS_CONTEXT.name, GM_IO_READ, &aws_config->state, sizeof(aws_config->state)) < 0) {
        memset(&aws_config->state, 0, sizeof(aws_config->state));
    }

    // Execute
    char *trail_title = NULL;
    gm_strcat(&trail_title, "Bucket:", ' ');
    gm_strcat(&trail_title, exec_bucket->aws_account_id, ' ');
    if(exec_bucket->aws_account_alias){
        gm_strcat(&trail_title, "(", '\0');
        gm_strcat(&trail_title, exec_bucket->aws_account_alias, '\0');
        gm_strcat(&trail_title, ")", '\0');
    }
    gm_strcat(&trail_title, " - ", ' ');

    mtdebug1(GM_AWS_LOGTAG, "Launching S3 Command: %s", command);

    const int gm_exec_ret_code = gm_exec(command, &output, &status, 0, NULL);

    os_free(command);

    if (gm_exec_ret_code != 0){
        mterror(GM_AWS_LOGTAG, "Internal error. Exiting...");
        os_free(trail_title);
        if (gm_exec_ret_code > 0) {
            os_free(output);
        }
        pthread_exit(NULL);
    } else if (status > 0) {
        mtwarn(GM_AWS_LOGTAG, "%s Returned exit code %d", trail_title, status);
        if(status == 1) {
            char * unknown_error_msg = strstr(output,"Unknown error");
            if (unknown_error_msg == NULL)
                mtwarn(GM_AWS_LOGTAG, "%s Unknown error.", trail_title);
            else
                mtwarn(GM_AWS_LOGTAG, "%s %s", trail_title, unknown_error_msg);
        } else if(status == 2) {
            char * ptr;
            if (ptr = strstr(output, "aws.py: error:"), ptr) {
                ptr += 14;
                mtwarn(GM_AWS_LOGTAG, "%s Error parsing arguments: %s", trail_title, ptr);
            } else {
                mtwarn(GM_AWS_LOGTAG, "%s Error parsing arguments.", trail_title);
            }
        } else {
            char * ptr;
            if (ptr = strstr(output, "ERROR: "), ptr) {
                ptr += 7;
                mtwarn(GM_AWS_LOGTAG, "%s %s", trail_title, ptr);
            } else {
                mtwarn(GM_AWS_LOGTAG, "%s %s", trail_title, output);
            }
        }
        mtdebug1(GM_AWS_LOGTAG, "%s OUTPUT: %s", trail_title, output);
    } else {
        mtdebug2(GM_AWS_LOGTAG, "%s OUTPUT: %s", trail_title, output);
    }

    char *line;
    char *save_ptr = NULL;
    for (line = strtok_r(output, "\n", &save_ptr); line; line = strtok_r(NULL, "\n", &save_ptr)) {
        gm_sendmsg(usec, aws_config->queue_fd, line, GM_AWS_CONTEXT.name, LOCALFILE_MQ);
    }

    os_free(trail_title);
    os_free(output);
}

// Run a service parsing

void gm_aws_run_service(gm_aws *aws_config, gm_aws_service *exec_service) {
    int status;
    char *output = NULL;
    char *command = NULL;

    // Define time to sleep between messages sent
    int usec = 1000000 / gm_max_eps;

    // Create arguments
    mtdebug2(GM_AWS_LOGTAG, "Create argument list");

    // script path
    char * script = NULL;
    os_calloc(PATH_MAX, sizeof(char), script);

    snprintf(script, PATH_MAX, "%s", GM_AWS_SCRIPT_PATH);

    gm_strcat(&command, script, '\0');
    os_free(script);

    // service
    gm_strcat(&command, "--service", ' ');
    gm_strcat(&command, exec_service->type, ' ');

    // service arguments
    if (exec_service->access_key) {
        gm_strcat(&command, "--access_key", ' ');
        gm_strcat(&command, exec_service->access_key, ' ');
    }
    if (exec_service->secret_key) {
        gm_strcat(&command, "--secret_key", ' ');
        gm_strcat(&command, exec_service->secret_key, ' ');
    }
    if (exec_service->aws_profile) {
        gm_strcat(&command, "--aws_profile", ' ');
        gm_strcat(&command, exec_service->aws_profile, ' ');
    }
    if (exec_service->iam_role_arn) {
        gm_strcat(&command, "--iam_role_arn", ' ');
        gm_strcat(&command, exec_service->iam_role_arn, ' ');
    }
    if (exec_service->iam_role_duration){
        gm_strcat(&command, "--iam_role_duration", ' ');
        gm_strcat(&command, exec_service->iam_role_duration, ' ');
    }
    if (exec_service->aws_account_id) {
        gm_strcat(&command, "--aws_account_id", ' ');
        gm_strcat(&command, exec_service->aws_account_id, ' ');
    }
    if (exec_service->aws_account_alias) {
        gm_strcat(&command, "--aws_account_alias", ' ');
        gm_strcat(&command, exec_service->aws_account_alias, ' ');
    }
    if (exec_service->only_logs_after) {
        gm_strcat(&command, "--only_logs_after", ' ');
        gm_strcat(&command, exec_service->only_logs_after, ' ');
    }
    if (exec_service->regions) {
        gm_strcat(&command, "--regions", ' ');
        gm_strcat(&command, exec_service->regions, ' ');
    }
    if (exec_service->aws_log_groups) {
        gm_strcat(&command, "--aws_log_groups", ' ');
        gm_strcat(&command, exec_service->aws_log_groups, ' ');
    }
    if (exec_service->remove_log_streams) {
        gm_strcat(&command, "--remove-log-streams", ' ');
    }
    if (exec_service->discard_field) {
        gm_strcat(&command, "--discard-field", ' ');
        gm_strcat(&command, exec_service->discard_field, ' ');
    }
    if (exec_service->discard_regex) {
        gm_strcat(&command, "--discard-regex", ' ');
        gm_strcat(&command, exec_service->discard_regex, ' ');
    }
    if (exec_service->sts_endpoint) {
        gm_strcat(&command, "--sts_endpoint", ' ');
        gm_strcat(&command, exec_service->sts_endpoint, ' ');
    }
    if (exec_service->service_endpoint) {
        gm_strcat(&command, "--service_endpoint", ' ');
        gm_strcat(&command, exec_service->service_endpoint, ' ');
    }
    if (isDebug()) {
        gm_strcat(&command, "--debug", ' ');
        if (isDebug() > 2) {
            gm_strcat(&command, "3", ' ');
        } else if (isDebug() > 1) {
            gm_strcat(&command, "2", ' ');
        } else {
            gm_strcat(&command, "1", ' ');
        }
    }
    if (aws_config->skip_on_error) {
        gm_strcat(&command, "--skip_on_error", ' ');
    }
    if (gm_state_io(GM_AWS_CONTEXT.name, GM_IO_READ, &aws_config->state, sizeof(aws_config->state)) < 0) {
        memset(&aws_config->state, 0, sizeof(aws_config->state));
    }

    // Execute
    char *service_title = NULL;
    gm_strcat(&service_title, "Service:", ' ');
    gm_strcat(&service_title, exec_service->type, ' ');
    gm_strcat(&service_title, exec_service->aws_account_id, ' ');
    if(exec_service->aws_account_alias){
        gm_strcat(&service_title, "(", '\0');
        gm_strcat(&service_title, exec_service->aws_account_alias, '\0');
        gm_strcat(&service_title, ")", '\0');
    }
    gm_strcat(&service_title, " - ", ' ');

    mtdebug1(GM_AWS_LOGTAG, "Launching S3 Command: %s", command);

    const int gm_exec_ret_code = gm_exec(command, &output, &status, 0, NULL);

    os_free(command);

    if (gm_exec_ret_code) {
        mterror(GM_AWS_LOGTAG, "Internal error. Exiting...");
        os_free(service_title);

        if (gm_exec_ret_code > 0) {
            os_free(output);
        }
        pthread_exit(NULL);
    } else if (status > 0) {
        mtwarn(GM_AWS_LOGTAG, "%s Returned exit code %d", service_title, status);
        if(status == 1) {
            char * unknown_error_msg = strstr(output,"Unknown error");
            if (unknown_error_msg == NULL)
                mtwarn(GM_AWS_LOGTAG, "%s Unknown error.", service_title);
            else
                mtwarn(GM_AWS_LOGTAG, "%s %s", service_title, unknown_error_msg);
        } else if(status == 2) {
            char * ptr;
            if (ptr = strstr(output, "aws.py: error:"), ptr) {
                ptr += 14;
                mtwarn(GM_AWS_LOGTAG, "%s Error parsing arguments: %s", service_title, ptr);
            } else {
                mtwarn(GM_AWS_LOGTAG, "%s Error parsing arguments.", service_title);
            }
        } else {
            char * ptr;
            if (ptr = strstr(output, "ERROR: "), ptr) {
                ptr += 7;
                mtwarn(GM_AWS_LOGTAG, "%s %s", service_title, ptr);
            } else {
                mtwarn(GM_AWS_LOGTAG, "%s %s", service_title, output);
            }
        }
        mtdebug1(GM_AWS_LOGTAG, "%s OUTPUT: %s", service_title, output);
    } else {
        mtdebug2(GM_AWS_LOGTAG, "%s OUTPUT: %s", service_title, output);
    }

    os_free(service_title);

    char *line;
    char *save_ptr = NULL;
    for (line = strtok_r(output, "\n", &save_ptr); line; line = strtok_r(NULL, "\n", &save_ptr)) {
        gm_sendmsg(usec, aws_config->queue_fd, line, GM_AWS_CONTEXT.name, LOCALFILE_MQ);
    }

    os_free(output);
}

// Run a subscriber parsing
void gm_aws_run_subscriber(gm_aws *aws_config, gm_aws_subscriber *exec_subscriber) {
    int status;
    char *output = NULL;
    char *command = NULL;

    // Define time to sleep between messages sent
    int usec = 1000000 / gm_max_eps;

    // Create arguments
    mtdebug2(GM_AWS_LOGTAG, "Create argument list");

    // script path
    char * script = NULL;
    os_calloc(PATH_MAX, sizeof(char), script);

    snprintf(script, PATH_MAX, "%s", GM_AWS_SCRIPT_PATH);

    gm_strcat(&command, script, '\0');
    os_free(script);

    // subscriber
    gm_strcat(&command, "--subscriber", ' ');
    gm_strcat(&command, exec_subscriber->type, ' ');

    gm_strcat(&command, "--queue", ' ');
    gm_strcat(&command, exec_subscriber->sqs_name, ' ');

    // subscriber arguments
    if (exec_subscriber->external_id) {
        gm_strcat(&command, "--external_id", ' ');
        gm_strcat(&command, exec_subscriber->external_id, ' ');
    }
    if (exec_subscriber->iam_role_arn) {
        gm_strcat(&command, "--iam_role_arn", ' ');
        gm_strcat(&command, exec_subscriber->iam_role_arn, ' ');
    }
    if (exec_subscriber->iam_role_duration){
        gm_strcat(&command, "--iam_role_duration", ' ');
        gm_strcat(&command, exec_subscriber->iam_role_duration, ' ');
    }
    if (exec_subscriber->aws_profile) {
        gm_strcat(&command, "--aws_profile", ' ');
        gm_strcat(&command, exec_subscriber->aws_profile, ' ');
    }
    if (exec_subscriber->sts_endpoint){
        gm_strcat(&command, "--sts_endpoint", ' ');
        gm_strcat(&command, exec_subscriber->sts_endpoint, ' ');
    }
    if (exec_subscriber->service_endpoint){
        gm_strcat(&command, "--service_endpoint", ' ');
        gm_strcat(&command, exec_subscriber->service_endpoint, ' ');
    }

    if (exec_subscriber->discard_field) {
        gm_strcat(&command, "--discard-field", ' ');
        gm_strcat(&command, exec_subscriber->discard_field, ' ');
    }
    if (exec_subscriber->discard_regex) {
        gm_strcat(&command, "--discard-regex", ' ');
        gm_strcat(&command, exec_subscriber->discard_regex, ' ');
    }

    if (isDebug()) {
        gm_strcat(&command, "--debug", ' ');
        if (isDebug() > 2) {
            gm_strcat(&command, "3", ' ');
        } else if (isDebug() > 1) {
            gm_strcat(&command, "2", ' ');
        } else {
            gm_strcat(&command, "1", ' ');
        }
    }

    if (aws_config->skip_on_error) {
        gm_strcat(&command, "--skip_on_error", ' ');
    }
    if (gm_state_io(GM_AWS_CONTEXT.name, GM_IO_READ, &aws_config->state, sizeof(aws_config->state)) < 0) {
        memset(&aws_config->state, 0, sizeof(aws_config->state));
    }

    // Execute
    char *subscriber_title = NULL;
    gm_strcat(&subscriber_title, "Subscriber:", ' ');
    gm_strcat(&subscriber_title, exec_subscriber->type, ' ');
    gm_strcat(&subscriber_title, exec_subscriber->sqs_name, ' ');

    gm_strcat(&subscriber_title, " - ", ' ');

    mtdebug1(GM_AWS_LOGTAG, "Launching S3 Subscriber Command: %s", command);

    const int gm_exec_ret_code = gm_exec(command, &output, &status, 0, NULL);

    os_free(command);

    if (gm_exec_ret_code) {
        mterror(GM_AWS_LOGTAG, "Internal error. Exiting...");
        os_free(subscriber_title);

        if (gm_exec_ret_code > 0) {
            os_free(output);
        }
        pthread_exit(NULL);
    } else if (status > 0) {
        mtwarn(GM_AWS_LOGTAG, "%s Returned exit code %d", subscriber_title, status);
        if(status == 1) {
            char * unknown_error_msg = strstr(output,"Unknown error");
            if (unknown_error_msg == NULL)
                mtwarn(GM_AWS_LOGTAG, "%s Unknown error.", subscriber_title);
            else
                mtwarn(GM_AWS_LOGTAG, "%s %s", subscriber_title, unknown_error_msg);
        } else if(status == 2) {
            char * ptr;
            if (ptr = strstr(output, "aws.py: error:"), ptr) {
                ptr += 14;
                mtwarn(GM_AWS_LOGTAG, "%s Error parsing arguments: %s", subscriber_title, ptr);
            } else {
                mtwarn(GM_AWS_LOGTAG, "%s Error parsing arguments.", subscriber_title);
            }
        } else {
            char * ptr;
            if (ptr = strstr(output, "ERROR: "), ptr) {
                ptr += 7;
                mtwarn(GM_AWS_LOGTAG, "%s %s", subscriber_title, ptr);
            } else {
                mtwarn(GM_AWS_LOGTAG, "%s %s", subscriber_title, output);
            }
        }
        mtdebug1(GM_AWS_LOGTAG, "%s OUTPUT: %s", subscriber_title, output);
    } else {
        mtdebug2(GM_AWS_LOGTAG, "%s OUTPUT: %s", subscriber_title, output);
    }

    os_free(subscriber_title);

    char *line;
    char *save_ptr = NULL;
    for (line = strtok_r(output, "\n", &save_ptr); line; line = strtok_r(NULL, "\n", &save_ptr)) {
        gm_sendmsg(usec, aws_config->queue_fd, line, GM_AWS_CONTEXT.name, LOCALFILE_MQ);
    }

    os_free(output);
}
