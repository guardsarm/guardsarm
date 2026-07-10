/*
 * GuardSarm Module for Agent Upgrading
 * Copyright (C) 2026 GuardSarm, Inc.
 * July 3, 2020.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifdef GUARDSARM_UNIT_TESTING
// Remove STATIC qualifier from tests
#define STATIC
#else
#define STATIC static
#endif

#include "wmodules.h"
#include "wm_agent_upgrade_tasks.h"
#include "wm_agent_upgrade_parsing.h"
#include "os_net.h"
#include "shared.h"

/* Hash table of current tasks based on agent_id */
STATIC OSHash *task_table_by_agent_id;

/**
 * Sends the task information locally to the task module queue
 * */
STATIC cJSON* gm_agent_send_task_information_master(const cJSON *message_object) __attribute__((nonnull));

/**
 * Sends a `send_sync` message into clusterd that will be received by the master node
 * */
STATIC cJSON* gm_agent_send_task_information_worker(const cJSON *message_object) __attribute__((nonnull));

gm_upgrade_task* gm_agent_upgrade_init_upgrade_task() {
    gm_upgrade_task *task;
    os_calloc(1, sizeof(gm_upgrade_task), task);
    return task;
}

gm_upgrade_custom_task* gm_agent_upgrade_init_upgrade_custom_task() {
    gm_upgrade_custom_task *task;
    os_calloc(1, sizeof(gm_upgrade_custom_task), task);
    return task;
}

gm_task_info* gm_agent_upgrade_init_task_info() {
    gm_task_info *task_info = NULL;
    os_calloc(1, sizeof(gm_task_info), task_info);
    return task_info;
}

gm_agent_info* gm_agent_upgrade_init_agent_info() {
    gm_agent_info *agent_info = NULL;
    os_calloc(1, sizeof(gm_agent_info), agent_info);
    return agent_info;
}

gm_agent_task* gm_agent_upgrade_init_agent_task() {
    gm_agent_task *agent_task = NULL;
    os_calloc(1, sizeof(gm_agent_task), agent_task);
    return agent_task;
}

gm_upgrade_agent_status_task* gm_agent_upgrade_init_agent_status_task() {
    gm_upgrade_agent_status_task *task = NULL;
    os_calloc(1, sizeof(gm_upgrade_agent_status_task), task);
    return task;
}

void gm_agent_upgrade_free_upgrade_task(gm_upgrade_task* upgrade_task) {
    if (upgrade_task) {
        os_free(upgrade_task->custom_version);
        os_free(upgrade_task->wpk_repository);
        os_free(upgrade_task->wpk_version);
        os_free(upgrade_task->wpk_file);
        os_free(upgrade_task->wpk_sha1);
        os_free(upgrade_task->package_type);
        os_free(upgrade_task);
    }
}

void gm_agent_upgrade_free_upgrade_custom_task(gm_upgrade_custom_task* upgrade_custom_task) {
    if (upgrade_custom_task) {
        os_free(upgrade_custom_task->custom_file_path);
        os_free(upgrade_custom_task->custom_installer);
        os_free(upgrade_custom_task);
    }
}

void gm_agent_upgrade_free_task_info(gm_task_info* task_info) {
    if (task_info) {
        if (task_info->task) {
            if (GM_UPGRADE_UPGRADE == task_info->command) {
                gm_agent_upgrade_free_upgrade_task((gm_upgrade_task*)task_info->task);
            } else if (GM_UPGRADE_UPGRADE_CUSTOM == task_info->command) {
                gm_agent_upgrade_free_upgrade_custom_task((gm_upgrade_custom_task*)task_info->task);
            }
        }
        os_free(task_info);
    }
}

void gm_agent_upgrade_free_agent_info(gm_agent_info* agent_info) {
    if (agent_info) {
        os_free(agent_info->platform);
        os_free(agent_info->major_version);
        os_free(agent_info->minor_version);
        os_free(agent_info->architecture);
        os_free(agent_info->guardsarm_version);
        os_free(agent_info->connection_status);
        os_free(agent_info->package_type);
        os_free(agent_info);
    }
}

void gm_agent_upgrade_free_agent_task(gm_agent_task* agent_task) {
    if (agent_task) {
        if (agent_task->agent_info) {
            gm_agent_upgrade_free_agent_info(agent_task->agent_info);
        }
        if (agent_task->task_info) {
            gm_agent_upgrade_free_task_info(agent_task->task_info);
        }
        os_free(agent_task);
    }
}

void gm_agent_upgrade_free_agent_status_task(gm_upgrade_agent_status_task* task) {
    if (task) {
        if (task->message) {
            os_free(task->message);
        }
        if (task->status) {
            os_free(task->status);
        }
        os_free(task);
    }
}

void gm_agent_upgrade_init_task_map() {
    task_table_by_agent_id = OSHash_Create();
}

void gm_agent_upgrade_destroy_task_map() {
    OSHash_Free(task_table_by_agent_id);
}

int gm_agent_upgrade_create_task_entry(int agent_id, gm_agent_task* agent_task) {
    char agent_id_string[128];
    sprintf(agent_id_string, "%d", agent_id);
    return OSHash_Add_ex(task_table_by_agent_id, agent_id_string, agent_task);
}

void gm_agent_upgrade_remove_entry(int agent_id, int free) {
    char agent_id_string[128];
    sprintf(agent_id_string, "%d", agent_id);
    gm_agent_task *agent_task = (gm_agent_task *)OSHash_Delete_ex(task_table_by_agent_id, agent_id_string);
    if (free) {
        gm_agent_upgrade_free_agent_task(agent_task);
    }
}

OSHashNode* gm_agent_upgrade_get_first_node(unsigned int *index) {
    return OSHash_Begin(task_table_by_agent_id, index);
}

OSHashNode* gm_agent_upgrade_get_next_node(unsigned int *index, OSHashNode *current) {
    return OSHash_Next(task_table_by_agent_id, index, current);
}

cJSON* gm_agent_upgrade_get_agent_ids() {
    OSHashNode *hash_node;
    unsigned int index = 0;
    cJSON *agents_array = cJSON_CreateArray();

    hash_node = OSHash_Begin(task_table_by_agent_id, &index);

    while(hash_node) {
        cJSON_AddItemToArray(agents_array, cJSON_CreateNumber(atoi(hash_node->key)));
        hash_node = OSHash_Next(task_table_by_agent_id, &index, hash_node);
    }
    if (!cJSON_GetArraySize(agents_array)) {
        cJSON_Delete(agents_array);
        return NULL;
    }

    return agents_array;
}

cJSON* gm_agent_upgrade_send_tasks_information(const cJSON *message_object) {
    if (w_is_worker()) {
        return gm_agent_send_task_information_worker(message_object);
    } else {
        return gm_agent_send_task_information_master(message_object);
    }
}

STATIC cJSON *gm_agent_send_task_information_master(const cJSON *message_object) {
    cJSON* response = NULL;

    int sock = OS_ConnectUnixDomain(GM_TASK_MODULE_SOCK, SOCK_STREAM, OS_MAXSTR);

    if (sock == OS_SOCKTERR) {
        mterror(GM_AGENT_UPGRADE_LOGTAG, GM_UPGRADE_UNREACHEABLE_TASK_MANAGER, GM_TASK_MODULE_SOCK);
    } else {
        char *buffer = NULL;
        int length;
        char *message = cJSON_PrintUnformatted(message_object);
        mtdebug1(GM_AGENT_UPGRADE_LOGTAG, GM_UPGRADE_TASK_SEND_MESSAGE, message);

        OS_SendSecureTCP(sock, strlen(message), message);
        os_free(message);
        os_calloc(OS_MAXSTR, sizeof(char), buffer);

        switch (length = OS_RecvSecureTCP(sock, buffer, OS_MAXSTR), length) {
            case OS_SOCKTERR:
                mterror(GM_AGENT_UPGRADE_LOGTAG, GM_UPGRADE_SOCKTERR_ERROR);
                break;
            case -1:
                mterror(GM_AGENT_UPGRADE_LOGTAG, GM_UPGRADE_RECV_ERROR, strerror(errno));
                break;
            default:
                response = cJSON_Parse(buffer);
                if (!response) {
                    mterror(GM_AGENT_UPGRADE_LOGTAG, GM_UPGRADE_INVALID_TASK_MAN_JSON);
                } else {
                    mtdebug1(GM_AGENT_UPGRADE_LOGTAG, GM_UPGRADE_TASK_RECEIVE_MESSAGE, buffer);
                }
                break;
        }
        os_free(buffer);

        close(sock);
    }

    return response;
}

STATIC cJSON *gm_agent_send_task_information_worker(const cJSON *message_object) {
    char response[OS_MAXSTR] = "";
    cJSON *message_duplicate = cJSON_Duplicate(message_object, 1);

    cJSON *payload = w_create_sendsync_payload(TASK_MANAGER_WM_NAME, message_duplicate);

    char *message = cJSON_PrintUnformatted(payload);

    mtdebug1(GM_AGENT_UPGRADE_LOGTAG, GM_UPGRADE_TASK_SEND_CLUSTER_MESSAGE, message);

    w_send_clustered_message("sendsync", message, response);

    if (response[0]) {
        mtdebug1(GM_AGENT_UPGRADE_LOGTAG, GM_UPGRADE_TASK_RECEIVE_MESSAGE, response);
    }

    os_free(message);
    cJSON_Delete(payload);

    return cJSON_Parse(response);
}
