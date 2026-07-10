/*
 * GuardSarm Module for Task management.
 * Copyright (C) 2026 GuardSarm, Inc.
 * July 13, 2020.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#include "wmodules.h"
#include "wm_task_manager_parsing.h"
#include "wm_task_manager_tasks.h"
#include "os_net.h"

#ifdef GUARDSARM_UNIT_TESTING
// Remove static qualifier when unit testing
#define STATIC

/* Replace pthread_exit with mock_assert, we do this to run some death tests on a very precarious way */
extern void mock_assert(const int result, const char* const expression,
                        const char * const file, const int line);

#define pthread_exit(x) mock_assert(0, #x, __FILE__, __LINE__)
#else
#define STATIC static
#endif


STATIC int gm_task_manager_init(gm_task_manager *task_config) __attribute__((nonnull));
STATIC void* gm_task_manager_main(gm_task_manager* task_config);    // Module main function. It won't return
STATIC void gm_task_manager_destroy(gm_task_manager* task_config);
STATIC void gm_task_manager_stop(gm_task_manager* task_config);
STATIC cJSON* gm_task_manager_dump(const gm_task_manager* task_config);

/* Context definition */
const gm_context GM_TASK_MANAGER_CONTEXT = {
    .name = TASK_MANAGER_WM_NAME,
    .start = (gm_routine)gm_task_manager_main,
    .destroy = (void (*)(void *))gm_task_manager_destroy,
    .dump = (cJSON * (*)(const void *))gm_task_manager_dump,
    .sync = NULL,
    .stop = (void (*)(void *))gm_task_manager_stop,
    .query = NULL,
};

size_t gm_task_manager_dispatch(const char *msg, char **response) {
    gm_task_manager_task *task = NULL;
    cJSON *json_response = NULL;
    cJSON *data_array = NULL;
    int error_code = GM_TASK_SUCCESS;

    mtdebug1(GM_TASK_MANAGER_LOGTAG, MOD_TASK_INCOMMING_MESSAGE, msg);

    // Parse message
    if (task = gm_task_manager_parse_message(msg), !task) {
        cJSON* parse_error = gm_task_manager_parse_data_response(GM_TASK_INVALID_MESSAGE, OS_INVALID, OS_INVALID, NULL);
        json_response = gm_task_manager_parse_response(GM_TASK_INVALID_MESSAGE, parse_error);
        *response = cJSON_PrintUnformatted(json_response);
        cJSON_Delete(json_response);
        return strlen(*response);
    }

    // Analyze task, update tasks DB and generate JSON response
    data_array = gm_task_manager_process_task(task, &error_code);

    switch (error_code) {
    case GM_TASK_INVALID_COMMAND:
        mterror(GM_TASK_MANAGER_LOGTAG, MOD_TASK_UNDEFINED_ACTION_ERRROR);
        cJSON_Delete(data_array);
        data_array = gm_task_manager_parse_data_response(GM_TASK_INVALID_COMMAND, OS_INVALID, OS_INVALID, NULL);
        break;
    case GM_TASK_DATABASE_ERROR:
        mterror(GM_TASK_MANAGER_LOGTAG, MOD_TASK_DB_ERROR);
        cJSON_Delete(data_array);
        data_array = gm_task_manager_parse_data_response(GM_TASK_DATABASE_ERROR, OS_INVALID, OS_INVALID, NULL);
        break;
    case GM_TASK_DATABASE_PARSE_ERROR:
        mterror(GM_TASK_MANAGER_LOGTAG, MOD_TASK_DB_ERROR);
        cJSON_Delete(data_array);
        data_array = gm_task_manager_parse_data_response(GM_TASK_DATABASE_PARSE_ERROR, OS_INVALID, OS_INVALID, NULL);
        break;
    case GM_TASK_DATABASE_REQUEST_ERROR:
        mterror(GM_TASK_MANAGER_LOGTAG, MOD_TASK_DB_ERROR);
        cJSON_Delete(data_array);
        data_array = gm_task_manager_parse_data_response(GM_TASK_DATABASE_REQUEST_ERROR, OS_INVALID, OS_INVALID, NULL);
        break;
    default:
        break;
    }

    json_response = gm_task_manager_parse_response(error_code, data_array);
    *response = cJSON_PrintUnformatted(json_response);

    mtdebug1(GM_TASK_MANAGER_LOGTAG, MOD_TASK_RESPONSE_MESSAGE, *response);

    gm_task_manager_free_task(task);
    cJSON_Delete(json_response);

    return strlen(*response);
}

STATIC int gm_task_manager_init(gm_task_manager *task_config) {
    int sock = 0;

    // Check if module is enabled
    if (!task_config->enabled) {
        mtinfo(GM_TASK_MANAGER_LOGTAG, MOD_TASK_DISABLED);
        pthread_exit(NULL);
    }

    // Start clean tasks thread
    w_create_thread(gm_task_manager_clean_tasks, task_config);

    /* Set the queue */
    if (sock = OS_BindUnixDomainWithPerms(TASK_QUEUE, SOCK_STREAM, OS_MAXSTR, getuid(), gm_getGroupID(), 0660), sock < 0) {
        mterror(GM_TASK_MANAGER_LOGTAG, MOD_TASK_CREATE_SOCK_ERROR, TASK_QUEUE, strerror(errno)); // LCOV_EXCL_LINE
        pthread_exit(NULL);
    }

    return sock;
}

STATIC void* gm_task_manager_main(gm_task_manager* task_config) {
    int sock;
    int peer;
    char *buffer = NULL;
    char *response = NULL;
    ssize_t length;
    fd_set fdset;

    if (w_is_worker()) {
        mtinfo(GM_TASK_MANAGER_LOGTAG, MOD_TASK_DISABLED_WORKER);
        return NULL;
    }

    // Initial configuration
    sock = gm_task_manager_init(task_config);

    mtinfo(GM_TASK_MANAGER_LOGTAG, STARTUP_MSG, (int)getpid());

    while (!gm_shutdown_requested) {

        switch (gm_select_interruptible(sock, &fdset)) {
        case -1:
            mterror(GM_TASK_MANAGER_LOGTAG, MOD_TASK_SELECT_ERROR, strerror(errno));
            pthread_exit(NULL);
            break;
        case 0:
            continue;
        default:
            break;
        }

        // Accept incomming connection
        if (peer = accept(sock, NULL, NULL), peer < 0) {
            if (errno != EINTR) {
                mterror(GM_TASK_MANAGER_LOGTAG, MOD_TASK_ACCEPT_ERROR, strerror(errno));
            }
            continue;
        }

        // Receive message from connection
        os_calloc(OS_MAXSTR, sizeof(char), buffer);
        switch (length = OS_RecvSecureTCP(peer, buffer, OS_MAXSTR), length) {
        case OS_SOCKTERR:
            mterror(GM_TASK_MANAGER_LOGTAG, MOD_TASK_SOCKTERR_ERROR);
            break;
        case -1:
            mterror(GM_TASK_MANAGER_LOGTAG, MOD_TASK_RECV_ERROR, strerror(errno));
            break;
        case 0:
            mtdebug1(GM_TASK_MANAGER_LOGTAG, MOD_TASK_EMPTY_MESSAGE);
            close(peer);
            break;
        case OS_MAXLEN:
            mterror(GM_TASK_MANAGER_LOGTAG, MOD_TASK_LENGTH_ERROR, MAX_DYN_STR);
            close(peer);
            break;
        default:
            length = gm_task_manager_dispatch(buffer, &response);
            // Send message to connection
            OS_SendSecureTCP(peer, length, response);
            os_free(response);
            close(peer);
        }
        os_free(buffer);

    #ifdef GUARDSARM_UNIT_TESTING
        break;
    #endif
    }

    close(sock);
    return NULL;
}

STATIC void gm_task_manager_stop(__attribute__((unused)) gm_task_manager* task_config) {
    gm_shutdown_requested = 1;
}

STATIC void gm_task_manager_destroy(gm_task_manager* task_config) {
    mtinfo(GM_TASK_MANAGER_LOGTAG, MOD_TASK_FINISH);
    os_free(task_config);
}

STATIC cJSON* gm_task_manager_dump(const gm_task_manager* task_config){
    cJSON *root = cJSON_CreateObject();
    cJSON *gm_info = cJSON_CreateObject();

    if (task_config->enabled) {
        cJSON_AddStringToObject(gm_info, "enabled", "yes");
    } else {
        cJSON_AddStringToObject(gm_info, "enabled", "no");
    }
    cJSON_AddItemToObject(root, "task-manager", gm_info);

    return root;
}
