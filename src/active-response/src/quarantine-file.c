/* Copyright (C) 2026 GuardSarm, Inc.
 * All right reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

/* Active response: quarantine-file — move a file into the agent's quarantine store
 * and neutralise it (strip all permissions). Message carries a top-level "path".
 * ENABLE quarantines; DISABLE restores the file to its original path. */

#include "active_responses.h"

#ifdef WIN32
#include <direct.h>
#define ar_mkdir(d) _mkdir(d)
#else
#include <sys/stat.h>
#define ar_mkdir(d) mkdir((d), 0700)
#endif

#define QUARANTINE_DIR "quarantine"

int main(int argc, char **argv) {
    (void)argc;
    char log_msg[OS_MAXSTR];
    char dest[OS_MAXSTR];
    int action = OS_INVALID;
    cJSON *input_json = NULL;

    action = setup_and_check_message(argv, &input_json);
    if ((action != ENABLE_COMMAND) && (action != DISABLE_COMMAND)) {
        return OS_INVALID;
    }

    const cJSON *path_json = cJSON_GetObjectItem(input_json, "path");
    const char *path = (path_json && cJSON_IsString(path_json)) ? path_json->valuestring : NULL;
    if (!path || !*path) {
        write_debug_file(argv[0], "Cannot read 'path' from data");
        cJSON_Delete(input_json);
        return OS_INVALID;
    }

    // basename (portable): last path separator
    const char *base = path;
    for (const char *p = path; *p; p++) {
        if (*p == '/' || *p == '\\') base = p + 1;
    }

    memset(dest, '\0', OS_MAXSTR);
    snprintf(dest, OS_MAXSTR - 1, "%s/%s.quarantined", QUARANTINE_DIR, base);

    if (action == ENABLE_COMMAND) {
        ar_mkdir(QUARANTINE_DIR);
        if (rename(path, dest) != 0) {
            memset(log_msg, '\0', OS_MAXSTR);
            snprintf(log_msg, OS_MAXSTR - 1, "Failed to quarantine '%s': %s", path, strerror(errno));
            write_debug_file(argv[0], log_msg);
            cJSON_Delete(input_json);
            return OS_INVALID;
        }
#ifndef WIN32
        chmod(dest, 0000);
#endif
        memset(log_msg, '\0', OS_MAXSTR);
        snprintf(log_msg, OS_MAXSTR - 1, "Quarantined '%s' -> '%s'", path, dest);
        write_debug_file(argv[0], log_msg);
    } else {
        // DISABLE: restore from quarantine to the original path
        if (rename(dest, path) != 0) {
            memset(log_msg, '\0', OS_MAXSTR);
            snprintf(log_msg, OS_MAXSTR - 1, "Failed to restore '%s': %s", path, strerror(errno));
            write_debug_file(argv[0], log_msg);
            cJSON_Delete(input_json);
            return OS_INVALID;
        }
        memset(log_msg, '\0', OS_MAXSTR);
        snprintf(log_msg, OS_MAXSTR - 1, "Restored '%s' from quarantine", path);
        write_debug_file(argv[0], log_msg);
    }

    write_debug_file(argv[0], "Ended");
    cJSON_Delete(input_json);
    return OS_SUCCESS;
}
