/*
 * GuardSarm Module for Security Configuration Assessment
 * Copyright (C) 2026 GuardSarm, Inc.
 * November 25, 2018.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef GM_GCP_H
#define GM_GCP_H

#define GM_GCP_PUBSUB_LOGTAG ARGV0 ":gcp-pubsub"
#define GM_GCP_BUCKET_LOGTAG ARGV0 ":gcp-bucket"
#define GM_GCP_SCRIPT_PATH "wodles/gcloud/gcloud"
#define GM_GCP_LOGGING_TOKEN ":gcloud_wodle:"

#define GM_GCP_DEF_INTERVAL 3600

typedef struct gm_gcp_pubsub {
    int enabled;
    int pull_on_start;
    int max_messages;
    int num_threads;
    time_t next_time;
    char *project_id;
    char *subscription_name;
    char *credentials_file;
    sched_scan_config scan_config;
} gm_gcp_pubsub;

typedef struct gm_gcp_bucket {
    char *bucket;                       // Bucket name
    char *type;                         // String defining bucket type.
    char *credentials_file;             // Path to the credentials file
    char *prefix;                       // Prefix or path to filter files
    char *only_logs_after;              // Date (YYYY-MMM-DD) to only parse logs after
    unsigned int remove_from_bucket:1;  // Remove the logs from the bucket
    struct gm_gcp_bucket *next;         // Pointer to next
} gm_gcp_bucket;

typedef struct gm_gcp_bucket_base {
    unsigned int enabled:1;
    unsigned int run_on_start:1;
    sched_scan_config scan_config;
    time_t next_time;                   // Absolute time for next scan
    gm_gcp_bucket *buckets;             // buckets (linked list)
} gm_gcp_bucket_base;

extern const gm_context GM_GCP_PUBSUB_CONTEXT;   // Context
extern const gm_context GM_GCP_BUCKET_CONTEXT;   // Context

/**
 * @brief Read the configuration for Google Cloud Pub/Sub
 * @param nodes XML nodes to analyze
 * @param module GuardSarm module to initialize
 */
int gm_gcp_pubsub_read(xml_node **nodes, gmodule *module);

/**
 * @brief Read the configuration for a Google Cloud bucket
 * @param nodes XML nodes to analyze
 * @param module GuardSarm module to initialize
 */
int gm_gcp_bucket_read(const OS_XML *xml, xml_node **nodes, gmodule *module);

#endif
