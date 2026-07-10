/*
 * GuardSarm Module for Microsoft Graph
 * Copyright (C) 2023, InfoDefense Inc.
 * March, 2023.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef GM_MS_GRAPH_H
#define GM_MS_GRAPH_H

#define GM_MS_GRAPH_LOGTAG ARGV0 ":" MS_GRAPH_WM_NAME

#define GM_MS_GRAPH_SCRIPT_PATH "wodles/ms_graph/ms-graph-logs"

#define GM_MS_GRAPH_DEFAULT_ENABLED true
#define GM_MS_GRAPH_DEFAULT_ONLY_FUTURE_EVENTS true
#define GM_MS_GRAPH_DEFAULT_CURL_MAX_SIZE 1048576L
#define GM_MS_GRAPH_DEFAULT_DELAY 30
#define GM_MS_GRAPH_DEFAULT_RUN_ON_START true
#define GM_MS_GRAPH_DEFAULT_VERSION "v1.0"

#define GM_MS_GRAPH_DEFAULT_TIMEOUT 60L
#define GM_MS_GRAPH_TIMESTAMP_SIZE_80 80
#define GM_MS_GRAPH_MAX_RETRIES      3
#define GM_MS_GRAPH_MAX_RETRY_AFTER_WARN  300
#define GM_MS_GRAPH_RETRY_AFTER_REGEX "Retry-After:\\s*(\\d+)"

#define GM_MS_GRAPH_GLOBAL_API_LOGIN_FQDN "login.microsoftonline.com"
#define GM_MS_GRAPH_GLOBAL_API_QUERY_FQDN "graph.microsoft.com"
#define GM_MS_GRAPH_GCC_HIGH_API_LOGIN_FQDN "login.microsoftonline.us"
#define GM_MS_GRAPH_GCC_HIGH_API_QUERY_FQDN "graph.microsoft.us"
#define GM_MS_GRAPH_DOD_API_LOGIN_FQDN "login.microsoftonline.us"
#define GM_MS_GRAPH_DOD_API_QUERY_FQDN "dod-graph.microsoft.us"


#define GM_MS_GRAPH_API_URL "https://%s/%s/%s/%s?$top=%d"
#define GM_MS_GRAPH_API_URL_FILTER_CREATED_DATE GM_MS_GRAPH_API_URL "&$filter=createdDateTime+ge+%s+and+createdDateTime+lt+%s"
#define GM_MS_GRAPH_API_URL_FILTER_ACTIVITY_DATE GM_MS_GRAPH_API_URL "&$filter=activityDateTime+ge+%s+and+activityDateTime+lt+%s"
#define GM_MS_GRAPH_ACCESS_TOKEN_URL "https://%s/%s/oauth2/v2.0/token"
#define GM_MS_GRAPH_ACCESS_TOKEN_PAYLOAD "scope=https://%s/.default&grant_type=client_credentials&client_id=%s&client_secret=%s"
#define GM_MS_GRAPH_ITEM_PER_PAGE 50

// MDM Intune
#define GM_MS_GRAPH_API_URL_DEVICES_EXPANDED "https://%s/%s/%s/%s/%s/%s?$top=%d"
#define GM_MS_GRAPH_API_URL_FILTER_DEVICE_FIELDS GM_MS_GRAPH_API_URL_DEVICES_EXPANDED "&$select=id,deviceName"
#define GM_MS_GRAPH_RESOURCE_DEVICE_MANAGEMENT "deviceManagement"
#define GM_MS_GRAPH_RELATIONSHIP_AUDIT_EVENTS "auditEvents"
#define GM_MS_GRAPH_RELATIONSHIP_MANAGED_DEVICES "managedDevices"
#define GM_MS_GRAPH_RELATIONSHIP_DETECTED_APPS "detectedApps"

// Identity protection
#define GM_MS_GRAPH_RESOURCE_IDENTITY_PROTECTION "identityProtection"
#define GM_MS_GRAPH_RELATIONSHIP_RISK_DETECTIONS "riskDetections"
#define GM_MS_GRAPH_RELATIONSHIP_SERVICE_PRINCIPAL_RISK_DETECTIONS "servicePrincipalRiskDetections"

typedef struct gm_ms_graph_state_t {
	time_t next_time;
} gm_ms_graph_state_t;

typedef struct gm_ms_graph_auth {
	char* client_id;
	char* tenant_id;
	char* secret_value;
	char* login_fqdn;
	char* query_fqdn;
	char* access_token;
	time_t token_expiration_time;
} gm_ms_graph_auth;

typedef struct gm_ms_graph_resource {
	char* name;
	char** relationships;
	unsigned int num_relationships;
} gm_ms_graph_resource;

typedef struct gm_ms_graph {
	bool enabled;
	bool only_future_events;
	ssize_t curl_max_size;
	unsigned int page_size;
	time_t time_delay;
	bool run_on_start;
	char* version;
	sched_scan_config scan_config;
	gm_ms_graph_auth **auth_config;
	gm_ms_graph_resource* resources;
	unsigned int num_resources;
	gm_ms_graph_state_t state;
} gm_ms_graph;

extern const gm_context GM_MS_GRAPH_CONTEXT; // Context

// Parse XML
int gm_ms_graph_read(const OS_XML* xml, xml_node** nodes, gmodule* module);

#endif // WM_MS_GRAPH_H
