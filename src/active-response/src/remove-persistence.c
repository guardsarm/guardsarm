/* Copyright (C) 2026 GuardSarm, Inc.
 * All right reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

/* Active response: remove-persistence — remove a persistence artifact by path.
 * Message carries a top-level "target" (the persistence file/entry path). ENABLE
 * removes the artifact (cron file, systemd unit, startup script, autostart entry);
 * DISABLE is a no-op (removal is not auto-reversible). */

#include "active_responses.h"

#ifndef WIN32
#include <sys/stat.h>
#endif

// Only allow removal under recognised persistence locations (defence-in-depth:
// never let a malformed message delete arbitrary system files).
static int is_persistence_path(const char *p) {
#ifdef WIN32
    return (strcasestr(p, "\\Startup\\") != NULL) ||
           (strcasestr(p, "\\Start Menu\\Programs\\Startup") != NULL) ||
           (strcasestr(p, "Microsoft\\Windows\\Start Menu") != NULL);
#else
    const char *dirs[] = {
        "/etc/cron", "/var/spool/cron", "/etc/systemd/system", "/lib/systemd/system",
        "/etc/init.d", "/etc/rc.local", "/etc/profile.d", "/etc/xdg/autostart",
        "/.config/autostart", "/.bashrc", "/.bash_profile", "/.profile", NULL
    };
    for (int i = 0; dirs[i]; i++) {
        if (strstr(p, dirs[i])) return 1;
    }
    return 0;
#endif
}

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
        write_debug_file(argv[0], "Ended (disable is a no-op for remove-persistence)");
        cJSON_Delete(input_json);
        return OS_SUCCESS;
    }

    const cJSON *t_json = cJSON_GetObjectItem(input_json, "target");
    const char *target = (t_json && cJSON_IsString(t_json)) ? t_json->valuestring : NULL;
    if (!target || !*target) {
        write_debug_file(argv[0], "Cannot read 'target' from data");
        cJSON_Delete(input_json);
        return OS_INVALID;
    }

    if (!is_persistence_path(target)) {
        memset(log_msg, '\0', OS_MAXSTR);
        snprintf(log_msg, OS_MAXSTR - 1, "Refusing: '%s' is not under a known persistence location", target);
        write_debug_file(argv[0], log_msg);
        cJSON_Delete(input_json);
        return OS_INVALID;
    }

    if (remove(target) != 0) {
        memset(log_msg, '\0', OS_MAXSTR);
        snprintf(log_msg, OS_MAXSTR - 1, "Failed to remove persistence '%s': %s", target, strerror(errno));
        write_debug_file(argv[0], log_msg);
        cJSON_Delete(input_json);
        return OS_INVALID;
    }

    memset(log_msg, '\0', OS_MAXSTR);
    snprintf(log_msg, OS_MAXSTR - 1, "Removed persistence artifact '%s'", target);
    write_debug_file(argv[0], log_msg);
    write_debug_file(argv[0], "Ended");

    cJSON_Delete(input_json);
    return OS_SUCCESS;
}
