/* Copyright (C) 2026 GuardSarm, Inc.
 * All right reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

/* Active response: kill-process — terminate a process by PID on the endpoint.
 * Message carries a top-level "pid" (and optional "process" name for logging).
 * Only ENABLE terminates; DISABLE is a no-op (a killed process cannot be revived). */

#include "active_responses.h"

#ifdef WIN32
#include <windows.h>
#else
#include <signal.h>
#endif

int main(int argc, char **argv) {
    (void)argc;
    char log_msg[OS_MAXSTR];
    int action = OS_INVALID;
    cJSON *input_json = NULL;

    action = setup_and_check_message(argv, &input_json);
    if ((action != ENABLE_COMMAND) && (action != DISABLE_COMMAND)) {
        return OS_INVALID;
    }
    if (action == DISABLE_COMMAND) {
        write_debug_file(argv[0], "Ended (disable is a no-op for kill-process)");
        cJSON_Delete(input_json);
        return OS_SUCCESS;
    }

    const cJSON *pid_json = cJSON_GetObjectItem(input_json, "pid");
    const char *pid_str = (pid_json && cJSON_IsString(pid_json)) ? pid_json->valuestring : NULL;
    if (!pid_str || !*pid_str) {
        write_debug_file(argv[0], "Cannot read 'pid' from data");
        cJSON_Delete(input_json);
        return OS_INVALID;
    }
    int pid = atoi(pid_str);
    if (pid <= 1) {
        write_debug_file(argv[0], "Refusing to kill an invalid or protected PID");
        cJSON_Delete(input_json);
        return OS_INVALID;
    }

    int ok = 0;
#ifdef WIN32
    HANDLE h = OpenProcess(PROCESS_TERMINATE, FALSE, (DWORD)pid);
    if (h) {
        ok = TerminateProcess(h, 1) ? 1 : 0;
        CloseHandle(h);
    }
#else
    ok = (kill((pid_t)pid, SIGKILL) == 0) ? 1 : 0;
#endif

    memset(log_msg, '\0', OS_MAXSTR);
    if (ok) {
        snprintf(log_msg, OS_MAXSTR - 1, "Process PID %d terminated", pid);
    } else {
        snprintf(log_msg, OS_MAXSTR - 1, "Failed to terminate PID %d", pid);
    }
    write_debug_file(argv[0], log_msg);
    write_debug_file(argv[0], "Ended");

    cJSON_Delete(input_json);
    return ok ? OS_SUCCESS : OS_INVALID;
}
