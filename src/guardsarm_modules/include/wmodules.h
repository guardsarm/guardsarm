/*
 * GuardSarm Module Manager
 * Copyright (C) 2026 GuardSarm, Inc.
 * April 22, 2016.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef W_MODULES
#define W_MODULES

#include "shared.h"
#include <pthread.h>
#include "config.h"
#include "wmodules_def.h"

#define GM_STATE_DIR    "var/wodles"               // Default directory for states.
#define GM_DIR_WIN      "wodles"                    // Default directory for states (Windows)
#define GM_STRING_MAX   67108864                    // Max. dynamic string size (64 MB).
#define GM_BUFFER_MAX   1024                        // Max. static buffer size.
#define GM_BUFFER_MIN   1024                        // Starting JSON buffer length.
#define GM_MAX_ATTEMPTS 3                           // Max. number of attempts.
#define GM_MAX_WAIT     500                           // Max. wait between attempts in milliseconds.
#define GM_IO_WRITE     0
#define GM_IO_READ      1
#define GM_ERROR_TIMEOUT 1                          // Error code for timeout.
#define VU_WM_NAME "vulnerability-detector"
#define AZ_WM_NAME "azure-logs"
#define SCA_WM_NAME "sca"
#define GCP_PUBSUB_WM_NAME "gcp-pubsub"
#define GCP_BUCKET_WM_NAME "gcp-bucket"
#define AGENT_UPGRADE_WM_NAME "agent-upgrade"
#define TASK_MANAGER_WM_NAME "task-manager"
#define GITHUB_WM_NAME "github"
#define OFFICE365_WM_NAME "office365"
#define MS_GRAPH_WM_NAME "ms-graph"
#define AGENT_INFO_WM_NAME   "agent-info"
#define SYSCOLLECTOR_WM_NAME "syscollector"
#define FIM_NAME "fim"                      // FIM module name. It is not a wm:module, but we define the name for query it.

#define GM_DEF_TIMEOUT      1800            // Default runtime limit (30 minutes)
#define GM_DEF_INTERVAL     86400           // Default cycle interval (1 day)

#define DAY_SEC    86400

#define RANDOM_LENGTH  512
#define MAX_VALUE_NAME 16383

#define EXECVE_ERROR 0x7F

// Verification type
typedef enum crypto_type {
    MD5SUM,
    SHA1SUM,
    SHA256SUM
} crypto_type;

// Inclusion of modules

#include "wm_database.h"
#include "wm_syscollector.h"
#include "wm_edr.h"
#include "wm_command.h"
#include "wm_aws.h"
#include "wm_azure.h"
#include "wm_docker.h"
#include "wm_sca.h"
#include "wm_control.h"
#include "wm_gcp.h"
#include "wm_task_general.h"
#include "wm_agent_upgrade.h"
#include "wm_task_manager.h"
#include "wm_agent_info.h"
#include "wm_github.h"
#include "wm_office365.h"
#include "wm_router.h"
#include "wm_content_manager.h"
#include "wm_vulnerability_scanner.h"
#include "wm_ms_graph.h"
#include "wm_inventory_sync.h"

extern gmodule *gmodules;       // Loaded modules.
extern int gm_task_nice;        // Nice value for tasks.
extern int gm_max_eps;          // Maximum events per second sent by GuardSarm Module
extern int gm_kill_timeout;     // Time for a process to quit before killing it
extern int gm_debug_level;
extern volatile sig_atomic_t gm_shutdown_requested;

void gm_sleep_interruptible(int seconds);
void gm_sleep_until_interruptible(time_t abs_time);
// Like select() on a single read fd but checks wm_shutdown_requested every second; returns 0 on shutdown.
int gm_select_interruptible(int sock, fd_set *fdset);

// Read XML configuration and internal options
int gm_config();
cJSON *getModulesConfig(void);
cJSON *getModulesInternalOptions(void);
int modulesSync(char* args, size_t length);

// Add module to the global list
void gm_add(gmodule *module);

/*
 * @brief Get ID group of GuardSarm user.
 *
 * @return ID group.
 */
gid_t gm_getGroupID(void);

/*
 * @brief Set ID group of guardsarm modules
 *
 * @param[in] gid ID group.
 */
void gm_setGroupID(const gid_t gid);

// Check general configuration
int gm_check();

// Destroy configuration data
void gm_destroy();

// Destroy module
void gm_module_free(gmodule * config);

/* Execute command with timeout of secs. exitcode can be NULL.
 *
 * command is a mutable string.
 * output is a pointer to dynamic string. Caller is responsible for freeing it!
 * On success, return 0. On another error, returns -1.
 * If the called program timed-out, returns WM_ERROR_TIMEOUT and output may
 * contain data.
 * env_path is a pointer to an string to add to the PATH environment variable.
 */
int gm_exec(char *command, char **output, int *exitcode, int secs, const char * add_path);

#ifdef WIN32
// Add process to pool
void gm_append_handle(HANDLE hProcess);

// Remove process group from pool
void gm_remove_handle(HANDLE hProcess);
#else
// Add process to pool
void gm_append_sid(pid_t sid);

// Remove process group from pool
void gm_remove_sid(pid_t sid);
#endif

// Initialize children pool
void gm_children_pool_init();

// Terminate every child process group
void gm_kill_children();

// Reads an HTTP header and extracts an element from a regex
char* gm_read_http_header_element(char *header, char *regex);

/* Load or save the running state
 * op: WM_IO_READ | WM_IO_WRITE
 * Returns 0 if success, or 1 if fail.
 */
int gm_state_io(const char * tag, int op, void *state, size_t size);

// Frees the wmodule struct
void gm_free(gmodule * c);

// Send message to a queue with a specific delay
int gm_sendmsg(int usec, int queue, const char *message, const char *locmsg, char loc) __attribute__((nonnull));

// Send message to a queue with a specific delay, and the option to stop the wait process.
int gm_sendmsg_ex(int usec, int queue, const char *message, const char *locmsg, char loc, bool (*fn_prd)()) __attribute__((nonnull));

/**
 Check the binary which executes a command has the specified hash.
 Returns:
     1 if the binary matches with the specified digest, 0 if not.
    -1 if the binary doesn't exist.
    -2 invalid parameters.
*/
int gm_validate_command(const char *command, const char *digest, crypto_type ctype);

#ifndef WIN32
// Com request thread dispatcher
void * wmcom_main(void * arg);
/**
 * @brief Send a one-way message to wmodules
 *
 * @param message Payload.
 * @param length Length in bytes of the input message
 */
#endif
void wmcom_send(char * message, size_t length);
size_t wmcom_dispatch(char * command, size_t length, char ** output);
size_t wmcom_getconfig(const char * section, char ** output);
int wmcom_sync(char * buffer, size_t length);

/**
 * @brief Find a module
 *
 * @param name Name of the module.
 * @return Pointer to a module structure.
 * @return NULL if the module was not found.
 */
gmodule * gm_find_module(const char * name);

/**
 * @brief Run a query in a module
 *
 * Run a command into a module structure, not in the same thread.
 * Query format: <module name> <command>
 *
 * @param query Command query
 * @param output Output payload
 * @return Size of the output
 */
size_t gm_module_query(char * query, char ** output);

/**
 * @brief Query module with JSON format (efficient version with module_name parameter)
 * @param module_name Name of the module to query
 * @param json_command JSON command string
 * @param output Output response
 * @return size_t Size of the response
 */
size_t gm_module_query_json_ex(const char* module_name, const char* json_command, char** output);

/**
 * @brief Query FIM module directly via syscom socket
 * @param command Command query
 * @param response Output response
 * @return size_t Size of the response
 */
size_t gm_fim_query_json(const char* command, char** response);

#endif // W_MODULES
