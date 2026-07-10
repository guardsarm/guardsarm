/*
 * GuardSarm Module Manager
 * Copyright (C) 2026 GuardSarm, Inc.
 * April 27, 2016.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#include "wmodules.h"
#include "module_query_errors.h"
#include "md5_op.h"
#include "sha1_op.h"
#include "sha256_op.h"
#include "os_net.h"
#include <sys/types.h>

#ifdef WIN32
// Forward declaration for syscom_dispatch function
size_t syscom_dispatch(char* command, size_t command_len, char** output);
#endif

gmodule *gmodules = NULL;   // Config: linked list of all modules.
int gm_task_nice = 0;       // Nice value for tasks.
static gid_t gm_gid;               // Group ID.
int gm_max_eps;             // Maximum events per second
int gm_kill_timeout;        // Time for a process to quit before killing it
int gm_debug_level;
volatile sig_atomic_t gm_shutdown_requested = 0;

void gm_sleep_interruptible(int seconds) {
    for (int i = 0; i < seconds && !gm_shutdown_requested; i++) {
        sleep(1);
    }
}

void gm_sleep_until_interruptible(time_t abs_time) {
    while (time(NULL) < abs_time && !gm_shutdown_requested) {
        sleep(1);
    }
}

int gm_select_interruptible(int sock, fd_set *fdset) {
    struct timeval tv;
    int r;
    while (!gm_shutdown_requested) {
        FD_ZERO(fdset);
        FD_SET(sock, fdset);
        tv = (struct timeval){ .tv_sec = 1, .tv_usec = 0 };
        r = select(sock + 1, fdset, NULL, NULL, &tv);
        if (r > 0) return r;
        if (r < 0 && errno != EINTR) return r;
    }
    return 0;
}


/**
 * List of modules that will be initialized by default
 * last position should be NULL
 * */
static const void *default_modules[] = {
    gm_agent_upgrade_read,
#ifndef CLIENT
    gm_task_manager_read,
#else
    gm_agent_info_read,
#endif

    NULL
};

/**
 * Initializes the default wmodules (will be enabled even if the wodle section for that module)
 * is not defined
 * @param wmodules pointer to wmodules array structure
 * @return a status flag
 * @retval OS_SUCCESS if all reading methods are executed successfully
 * @retval OS_INVALID if there is an error
 * */
static int gm_initialize_default_modules(gmodule **gmodules);

// Read XML configuration and internal options

gid_t gm_getGroupID(void)
{
    return gm_gid;
}

void gm_setGroupID(const gid_t gid)
{
    gm_gid = gid;
}

int gm_config() {

    int agent_cfg = 0;

    // Get defined values from internal_options

#ifdef CLIENT
    gm_task_nice = getDefine_Int("guardsarm_modules", "task_nice", -20, 19);
    gm_max_eps = getDefine_Int("guardsarm_modules", "max_eps", 1, 1000);
    gm_kill_timeout = getDefine_Int("guardsarm_modules", "kill_timeout", 0, 3600);
#else
    gm_task_nice = getDefine_Int_default("guardsarm_modules", "task_nice", -20, 19, 10);
    gm_max_eps = getDefine_Int_default("guardsarm_modules", "max_eps", 1, 1000, 100);
    gm_kill_timeout = getDefine_Int_default("guardsarm_modules", "kill_timeout", 0, 3600, 10);
#endif

    if(gm_initialize_default_modules(&gmodules) < 0) {
        return OS_INVALID;
    }


    // Read configuration

    if (ReadConfig(CWMODULE, GUARDSARMCONF, &gmodules, &agent_cfg) < 0) {
        return -1;
    }

#ifdef CLIENT
    // Read configuration: agent.conf
    agent_cfg = 1;
    ReadConfig(CWMODULE | CAGENT_CONFIG, AGENTCONFIG, &gmodules, &agent_cfg);
#else
    gmodule *module;

    if ((module = gm_router_read())) {
        gm_add(module);
    }

    if ((module = gm_content_manager_read())) {
        gm_add(module);
    }

    // Inventory sync
    if ((module = gm_inventory_sync_read()))
        gm_add(module);

    // The database module won't be available on agents
    if ((module = gm_database_read()))
        gm_add(module);

#endif

#if defined(__linux__) || defined(__MACH__) || defined(FreeBSD) || defined(OpenBSD)
    gmodule * control_module;
    control_module = gm_control_read();
    gm_add(control_module);
#endif

    return 0;
}

// Add module to the global list

void gm_add(gmodule *module) {
    gmodule *current;

    if (gmodules) {
        for (current = gmodules; current->next; current = current->next);
        current->next = module;
    } else
        gmodules = module;
}

// Check general configuration

int gm_check() {
    gmodule *i = gmodules;
    gmodule *j;
    gmodule *next;
    gmodule *prev = gmodules;

    // Discard empty configurations

    while (i) {
        if (i->context) {
            prev = i;
            i = i->next;
        } else {
            next = i->next;
            free(i);

            if (i == gmodules) {
                gmodules = next;
            } else {
                prev->next = next;
            }

            i = next;
        }
    }

    // Check that a configuration exists

    if (!gmodules) {
        return -1;
    }

    // Get the last module of the same type

#ifndef __clang_analyzer__
    for (i = gmodules->next; i; i = i->next) {
        for (j = prev = gmodules; j != i; j = next) {
            next = j->next;

            if (i->tag && j->tag && !strcmp(i->tag, j->tag)) {

                mdebug1("Deleting repeated module '%s'.", j->tag);

                if (j == gmodules) {
                    gmodules = prev = next;
                } else {
                    prev->next = next;
                }
                gm_module_free(j);

            } else {
                prev = j;
            }
        }
    }
#endif

    return 0;
}

// Destroy configuration data

void gm_destroy() {
    gm_free(gmodules);
}

// Load or save the running state

int gm_state_io(const char * tag, int op, void *state, size_t size) {
    char path[PATH_MAX] = { '\0' };
    size_t nmemb;
    FILE *file;

    #ifdef WIN32
    snprintf(path, PATH_MAX, "%s\\%s", GM_DIR_WIN, tag);
    #else
    snprintf(path, PATH_MAX, "%s/%s", GM_STATE_DIR, tag);
    #endif

    if (!(file = wfopen(path, op == GM_IO_WRITE ? "wb" : "rb"))) {
        return -1;
    }
    w_file_cloexec(file);

    nmemb = (op == GM_IO_WRITE) ? fwrite(state, size, 1, file) : fread(state, size, 1, file);
    fclose(file);

    return nmemb - 1;
}

char* gm_read_http_header_element(char *header, char *regex) {
    char *element = NULL;
    OSRegex os_regex;

    if (!header || !regex) {
        merror("Missing parameters.");
        return NULL;
    }

    if (!OSRegex_Compile(regex, &os_regex, OS_RETURN_SUBSTRING)) {
        mwarn("Cannot compile regex.");
        return NULL;
    }

    if (!OSRegex_Execute(header, &os_regex)) {
        mdebug1("No match regex.");
        OSRegex_FreePattern(&os_regex);
        return NULL;
    }

    if (!os_regex.d_sub_strings[0]) {
        mdebug1("No element was captured.");
        OSRegex_FreePattern(&os_regex);
        return NULL;
    }

    os_strdup(os_regex.d_sub_strings[0], element);

    // Trim at \r or \n to prevent matching across HTTP header boundaries
    char *cr = strchr(element, '\r');
    char *lf = strchr(element, '\n');
    if (cr) *cr = '\0';
    if (lf) *lf = '\0';

    OSRegex_FreePattern(&os_regex);
    return element;
}

void gm_free(gmodule * config) {
    gmodule *cur_module;
    gmodule *next_module;

    for (cur_module = config; cur_module; cur_module = next_module) {
        next_module = cur_module->next;

        gm_module_free(cur_module);
    }
}


void gm_module_free(gmodule * config){
    if (config->context && config->context->destroy)
            config->context->destroy(config->data);

    free(config->tag);
    free(config);
}


// Get read data
cJSON *getModulesConfig(void) {

    gmodule *cur_module;

    cJSON *root = cJSON_CreateObject();
    cJSON *gm_mod = cJSON_CreateArray();

    for (cur_module = gmodules; cur_module; cur_module = cur_module->next) {
        if (cur_module->context->dump) {
            cJSON * item = cur_module->context->dump(cur_module->data);

            if (item) {
                cJSON_AddItemToArray(gm_mod, item);
            }
        }
    }

    cJSON_AddItemToObject(root,"wmodules",gm_mod);

    return root;
}

// sync data
int modulesSync(char* args, size_t length) {
    int ret = -1;
    gmodule *cur_module = NULL;
    int retry = 0;

    do {
        if (retry > 0) {
            usleep(retry * GM_MAX_WAIT);
            mdebug1("WModules is not ready. Retry %d", retry);
        }

        for (cur_module = gmodules; cur_module; cur_module = cur_module->next) {
            if (strstr(args, cur_module->context->name)) {
                ret = 0;
                if (strstr(args, "_sync ") && cur_module->context->sync != NULL) {
                    ret = cur_module->context->sync(args, length);
                }
                break;
            }
        }

        ++retry;

        if (retry > GM_MAX_ATTEMPTS) {
            break;
        }
    } while (ret != 0);

    if (ret) {
        merror("At modulesSync(): Unable to sync module '%s': (%d)", cur_module ? cur_module->tag : "",  ret);
    }
    return ret;
}

// Find a module

gmodule * gm_find_module(const char * name) {
    for (gmodule * module = gmodules; module != NULL; module = module->next) {
        if (strcmp(module->context->name, name) == 0) {
            return module;
        }
    }

    return NULL;
}

// Run a query in a module

size_t gm_module_query(char * query, char ** output) {
    char * module_name = query;
    char * args = strchr(query, ' ');

    if (args == NULL) {
        os_strdup("err {\"error\":1,\"message\":\"Module query needs arguments\"}", *output);
        return strlen(*output);
    }

    *args++ = '\0';

    gmodule * module = gm_find_module(module_name);
    if (module == NULL) {
        os_strdup("err {\"error\":2,\"message\":\"Module not found or not configured\"}", *output);
        return strlen(*output);
    }

    if (module->context->query == NULL) {
        os_strdup("err {\"error\":3,\"message\":\"This module does not support queries\"}", *output);
        return strlen(*output);
    }

    return module->context->query(module->data, args, output);
}

// Query module with JSON format
// Internal implementation that accepts module_name directly
static size_t gm_module_query_json_internal(const char* module_name, const char* json_command, char** output) {
    cJSON *json_obj = NULL;
    cJSON *command_item = NULL;
    char error_msg[512];

    if (!module_name || !json_command || !output) {
        return 0;
    }

    // Parse JSON command
    json_obj = cJSON_Parse(json_command);
    if (!json_obj) {
        snprintf(error_msg, sizeof(error_msg), "{\"error\":%d,\"message\":\"%s\"}",
                 MQ_ERR_INVALID_JSON, MQ_MSG_INVALID_JSON);
        os_strdup(error_msg, *output);
        return strlen(*output);
    }

    // Validate command field exists
    command_item = cJSON_GetObjectItem(json_obj, "command");
    if (!command_item || !cJSON_IsString(command_item)) {
        cJSON_Delete(json_obj);
        snprintf(error_msg, sizeof(error_msg), "{\"error\":%d,\"message\":\"%s\"}",
                 MQ_ERR_INVALID_PARAMS, MQ_MSG_INVALID_PARAMS);
        os_strdup(error_msg, *output);
        return strlen(*output);
    }

    // Find the module
    gmodule * module = gm_find_module(module_name);
    if (module == NULL) {
        cJSON_Delete(json_obj);
        snprintf(error_msg, sizeof(error_msg), "{\"error\":%d,\"message\":\"%s\"}",
                 MQ_ERR_MODULE_NOT_FOUND, MQ_MSG_MODULE_NOT_FOUND);
        os_strdup(error_msg, *output);
        return strlen(*output);
    }

    if (module->context->query == NULL) {
        cJSON_Delete(json_obj);
        snprintf(error_msg, sizeof(error_msg), "{\"error\":%d,\"message\":\"%s\"}",
                 MQ_ERR_MODULE_NOT_SUPPORTED, MQ_MSG_MODULE_NOT_SUPPORTED);
        os_strdup(error_msg, *output);
        return strlen(*output);
    }

    // For SCA and Syscollector, pass the JSON command directly (it already contains all needed fields)
    if (strcmp(module_name, SCA_WM_NAME) == 0 || strcmp(module_name, SYSCOLLECTOR_WM_NAME) == 0) {
        // Pass the original JSON directly - no need to reconstruct
        // Cast to non-const as required by the query function signature
        size_t result = module->context->query(module->data, (char*)json_command, output);
        cJSON_Delete(json_obj);
        return result;
    }

    return 0;
}

// New external interface that accepts module_name as parameter
size_t gm_module_query_json_ex(const char* module_name, const char* json_command, char** output) {
    return gm_module_query_json_internal(module_name, json_command, output);
}

// Query FIM module directly via syscom socket with JSON format
size_t gm_fim_query_json(const char* command, char** response) {
    char error_msg[512];

    if (!command || !response) {
        return 0;
    }

    // Use the command directly as JSON (it's already in the new format)
    char *json_string = strdup(command);

#ifndef WIN32
    int sock;
    char *response_buffer = NULL;
    ssize_t recv_length;

    // Connect to syscheck syscom socket
    mdebug1("WM_FIM_QUERY_JSON: Attempting to connect to socket: %s", SYS_LOCAL_SOCK);
    if ((sock = OS_ConnectUnixDomain(SYS_LOCAL_SOCK, SOCK_STREAM, OS_MAXSTR)) < 0) {
        merror("WM_FIM_QUERY_JSON: Failed to connect to socket, errno=%d", errno);
        switch (errno) {
            case ECONNREFUSED:
                snprintf(error_msg, sizeof(error_msg), "{\"error\":%d,\"message\":\"Syscheck module refused connection. The component might be disabled\"}",
                         MQ_ERR_MODULE_NOT_RUNNING);
                break;
            case ENOENT:
                snprintf(error_msg, sizeof(error_msg), "{\"error\":%d,\"message\":\"Syscheck socket not found. The component might not be running\"}",
                         MQ_ERR_MODULE_NOT_RUNNING);
                break;
            default:
                snprintf(error_msg, sizeof(error_msg), "{\"error\":%d,\"message\":\"Could not connect to syscheck socket\"}",
                         MQ_ERR_MODULE_NOT_RUNNING);
        }
        os_strdup(error_msg, *response);
        os_free(json_string);
        return strlen(*response);
    }

    mdebug1("WM_FIM_QUERY_JSON: Sending JSON command: %s", json_string);

    // Send the JSON query to syscheck
    if (OS_SendSecureTCP(sock, strlen(json_string), json_string) < 0) {
        merror("WM_FIM_QUERY_JSON: Failed to send command");
        close(sock);
        snprintf(error_msg, sizeof(error_msg), "{\"error\":%d,\"message\":\"Could not send query to syscheck\"}",
                 MQ_ERR_INTERNAL);
        os_strdup(error_msg, *response);
        os_free(json_string);
        return strlen(*response);
    }

    // Allocate buffer for response
    os_calloc(OS_MAXSTR, sizeof(char), response_buffer);

    // Receive response from syscheck
    recv_length = OS_RecvSecureTCP(sock, response_buffer, OS_MAXSTR);
    switch (recv_length) {
        case OS_SOCKTERR:
            close(sock);
            os_free(response_buffer);
            snprintf(error_msg, sizeof(error_msg), "{\"error\":%d,\"message\":\"Response from syscheck too large\"}",
                     MQ_ERR_INTERNAL);
            os_strdup(error_msg, *response);
            break;
        case -1:
            close(sock);
            os_free(response_buffer);
            snprintf(error_msg, sizeof(error_msg), "{\"error\":%d,\"message\":\"Error receiving response from syscheck\"}",
                     MQ_ERR_INTERNAL);
            os_strdup(error_msg, *response);
            break;
        case 0:
            close(sock);
            os_free(response_buffer);
            snprintf(error_msg, sizeof(error_msg), "{\"error\":%d,\"message\":\"Empty response from syscheck\"}",
                     MQ_ERR_INTERNAL);
            os_strdup(error_msg, *response);
            break;
        default:
            close(sock);
            *response = response_buffer;
            break;
    }
#else
    // On Windows, use the syscom_dispatch function directly
    size_t response_len = syscom_dispatch(json_string, strlen(json_string), response);

    if (response_len == 0 || !(*response)) {
        snprintf(error_msg, sizeof(error_msg), "{\"error\":%d,\"message\":\"No response from syscom\"}",
                 MQ_ERR_INTERNAL);
        os_strdup(error_msg, *response);
    }
#endif

    os_free(json_string);
    return strlen(*response);
}

cJSON *getModulesInternalOptions(void) {

    cJSON *root = cJSON_CreateObject();
    cJSON *internals = cJSON_CreateObject();

    cJSON_AddNumberToObject(internals,"guardsarm_modules.task_nice",gm_task_nice);
    cJSON_AddNumberToObject(internals,"guardsarm_modules.max_eps",gm_max_eps);
    cJSON_AddNumberToObject(internals,"guardsarm_modules.kill_timeout",gm_kill_timeout);
    cJSON_AddNumberToObject(internals,"guardsarm_modules.debug",gm_debug_level);

    cJSON_AddItemToObject(root,"internal_options",internals);

    return root;
}

// Send message to a queue waiting for a specific delay
int gm_sendmsg(int usec, int queue, const char *message, const char *locmsg, char loc) {

#ifdef WIN32
    int msec = usec / 1000;
    Sleep(msec);
#else
    struct timeval timeout = {0, usec};
    select(0, NULL, NULL, NULL, &timeout);
#endif

    if (SendMSG(queue, message, locmsg, loc) < 0) {
        mdebug1("Unable to send message to queue: (%s)", strerror(errno));
        return -1;
    }

    return 0;
}

// Send message to a queue waiting for a specific delay
int gm_sendmsg_ex(int usec, int queue, const char *message, const char *locmsg, char loc, bool (*fn_prd)()) {

#ifdef WIN32
    int msec = usec / 1000;
    Sleep(msec);
#else
    struct timeval timeout = {0, usec};
    select(0, NULL, NULL, NULL, &timeout);
#endif

    if (SendMSGPredicated(queue, message, locmsg, loc, fn_prd) < 0) {
        mdebug1("Unable to send message to queue: (%s)", strerror(errno));
        return -1;
    }

    return 0;
}

/**
 Check the binary which executes a command has the specified hash.
 Returns:
     1 if the binary matches with the specified digest, 0 if not.
    -1 invalid parameters.
*/
int gm_validate_command(const char *command, const char *digest, crypto_type ctype) {

    os_md5 md5_binary;
    os_sha1 sha1_binary;
    os_sha256 sha256_binary;
    int match = 0;

    if (command == NULL || digest == NULL) {
        return -1;
    }

    switch (ctype) {

        case MD5SUM:
            // Get binary MD5
            OS_MD5_File(command, md5_binary, OS_BINARY);
            // Compare MD5 sums
            mdebug2("Comparing MD5 hash: '%s' | '%s'", md5_binary, digest);
            if (strcasecmp(md5_binary, digest) == 0) {
                match = 1;
            }
            break;

        case SHA1SUM:
            // Get binary SHA1
            OS_SHA1_File(command, sha1_binary, OS_BINARY);
            // Compare SHA1 sums
            mdebug2("Comparing SHA1 hash: '%s' | '%s'", sha1_binary, digest);
            if (strcasecmp(sha1_binary, digest) == 0) {
                match = 1;
            }
            break;

        case SHA256SUM:
            // Get binary SHA256
            OS_SHA256_File(command, sha256_binary, OS_BINARY);
            // Compare SHA256 sums
            mdebug2("Comparing SHA256 hash: '%s' | '%s'", sha256_binary, digest);
            if (strcasecmp(sha256_binary, digest) == 0) {
                match = 1;
            }
    }

    return match;
}

static int gm_initialize_default_modules(gmodule **gmodules) {
    gmodule *cur_wmodule = *gmodules;
    int i=0;
    while (default_modules[i]) {
        if(!cur_wmodule) {
            *gmodules = cur_wmodule = calloc(1, sizeof(gmodule));
        } else {
            os_calloc(1, sizeof(gmodule), cur_wmodule->next);
            cur_wmodule = cur_wmodule->next;
            if (!cur_wmodule) {
                merror(MEM_ERROR, errno, strerror(errno));
                return (OS_INVALID);
            }
        }
        // Point to read function
        int (*function_ptr)(const OS_XML *xml, xml_node **nodes, gmodule *module) = default_modules[i];

        if(function_ptr(NULL, NULL, cur_wmodule) == OS_INVALID) {
            return OS_INVALID;
        }
        i++;
    }
    return OS_SUCCESS;
}
