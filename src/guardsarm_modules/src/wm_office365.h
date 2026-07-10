/*
 * GuardSarm Module for Office365 events
 * Copyright (C) 2026 GuardSarm, Inc.
 * May 18, 2021.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef GM_OFFICE365_H
#define GM_OFFICE365_H

#define GM_OFFICE365_LOGTAG ARGV0 ":" OFFICE365_WM_NAME

#define GM_OFFICE365_DEFAULT_ENABLED 1
#define GM_OFFICE365_DEFAULT_ONLY_FUTURE_EVENTS 1
#define GM_OFFICE365_DEFAULT_INTERVAL 60
#define GM_OFFICE365_DEFAULT_CURL_MAX_SIZE 1048576L
#define GM_OFFICE365_DEFAULT_CURL_REQUEST_TIMEOUT 60L
#define GM_OFFICE365_DEFAULT_API_LOGIN_FQDN "login.microsoftonline.com"
#define GM_OFFICE365_DEFAULT_API_MANAGEMENT_FQDN "manage.office.com"

#define GM_OFFICE365_GCC_API_LOGIN_FQDN "login.microsoftonline.com"
#define GM_OFFICE365_GCC_API_MANAGEMENT_FQDN "manage-gcc.office.com"

#define GM_OFFICE365_GCC_HIGH_API_LOGIN_FQDN "login.microsoftonline.us"
#define GM_OFFICE365_GCC_HIGH_API_MANAGEMENT_FQDN "manage.office365.us"

#define GM_OFFICE365_MSG_DELAY 1000000 / gm_max_eps
#define GM_OFFICE365_RETRIES_TO_SEND_ERROR 3
#define GM_OFFICE365_NEXT_PAGE_REGEX "NextPageUri:\\s*(\\S+)"

#define GM_OFFICE365_API_ACCESS_TOKEN_URL "https://%s/%s/oauth2/v2.0/token"
#define GM_OFFICE365_API_SUBSCRIPTION_URL "https://%s/api/v1.0/%s/activity/feed/subscriptions/%s?contentType=%s"
#define GM_OFFICE365_API_CONTENT_BLOB_URL "https://%s/api/v1.0/%s/activity/feed/subscriptions/content?contentType=%s&startTime=%s&endTime=%s"

#define GM_OFFICE365_API_ACCESS_TOKEN_PAYLOAD "client_id=%s&scope=https://%s/.default&grant_type=client_credentials&client_secret=%s"

#define GM_OFFICE365_API_SUBSCRIPTION_START "start"
#define GM_OFFICE365_API_SUBSCRIPTION_STOP "stop"

typedef struct gm_office365_auth {
    char *tenant_id;
    char *client_id;
    char *client_secret_path;
    char *client_secret;
    char *login_fqdn;
    char *management_fqdn;
    struct gm_office365_auth *next;
} gm_office365_auth;

typedef struct gm_office365_subscription {
    char *subscription_name;
    struct gm_office365_subscription *next;
} gm_office365_subscription;

typedef struct gm_office365_state {
    time_t last_log_time;
} gm_office365_state;

typedef struct gm_office365_fail {
    int fails;
    char *tenant_id;
    char *subscription_name;
    struct gm_office365_fail *next;
} gm_office365_fail;

typedef struct gm_office365 {
    int enabled;
    int only_future_events;
    time_t interval;                        // Interval betweeen events in seconds
    ssize_t curl_max_size;
    gm_office365_auth *auth;
    gm_office365_subscription *subscription;
    gm_office365_fail *fails;
    int queue_fd;
} gm_office365;

extern const gm_context GM_OFFICE365_CONTEXT;  // Context

// Parse XML
int gm_office365_read(const OS_XML *xml, xml_node **nodes, gmodule *module);

#endif
