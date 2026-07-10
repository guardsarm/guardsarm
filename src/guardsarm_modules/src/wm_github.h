/*
 * GuardSarm Module for GitHub logs
 * Copyright (C) 2026 GuardSarm, Inc.
 * May 3, 2021.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef GM_GITHUB_H
#define GM_GITHUB_H

#define GM_GITHUB_LOGTAG ARGV0 ":" GITHUB_WM_NAME

#define GM_GITHUB_DEFAULT_ENABLED 1
#define GM_GITHUB_DEFAULT_ONLY_FUTURE_EVENTS 1
#define GM_GITHUB_DEFAULT_INTERVAL 60
#define GM_GITHUB_DEFAULT_DELAY 30
#define GM_GITHUB_MSG_DELAY 1000000 / gm_max_eps
#define GM_GITHUB_DEFAULT_CURL_MAX_SIZE 1048576L
#define GM_GITHUB_DEFAULT_CURL_REQUEST_TIMEOUT 60L

#define ITEM_PER_PAGE 100
#define RETRIES_TO_SEND_ERROR 3
#define GITHUB_NEXT_PAGE_REGEX "<(\\S+)>;\\s*rel=\"next\""

#define GITHUB_API_URL "https://api.github.com/orgs/%s/audit-log?phrase=created:%s..%s&include=%s&order=asc&per_page=%d"

#define EVENT_TYPE_ALL "all"
#define EVENT_TYPE_GIT "git"
#define EVENT_TYPE_WEB "web"

typedef struct gm_github_auth {
    char *org_name;                         // Organization name
    char *api_token;                        // Personal access token
    struct gm_github_auth *next;
} gm_github_auth;

typedef struct gm_github_state {
    time_t last_log_time;                      // Absolute time of last scan
} gm_github_state;

typedef struct gm_github_fail {
    int fails;
    char *org_name;
    char *event_type;
    struct gm_github_fail *next;
} gm_github_fail;

typedef struct gm_github {
    int enabled;
    int only_future_events;
    time_t interval;                        // Interval betweeen events in seconds
    time_t time_delay;
    ssize_t curl_max_size;
    gm_github_auth *auth;
    char *event_type;                       // Event types to include: web/git/all
    gm_github_fail *fails;
    int queue_fd;
} gm_github;

extern const gm_context GM_GITHUB_CONTEXT;  // Context

// Parse XML
int gm_github_read(const OS_XML *xml, xml_node **nodes, gmodule *module);

#endif
