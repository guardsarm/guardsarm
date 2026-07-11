/* Copyright (C) 2026 GuardSarm, Inc.
 * All right reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

/* Active response: collect-forensic — capture a triage snapshot of the endpoint
 * (process list, network connections, logged-in users) into active-response/forensics.
 * Message may carry a top-level "target" label. ENABLE collects; DISABLE is a no-op. */

#include "active_responses.h"

#ifndef WIN32
#include <sys/stat.h>
#include <time.h>
#endif

#define FORENSIC_DIR "forensics"

static void run_into(const char *bin_name, char *const args[], FILE *out, const char *title) {
    char *path = NULL;
    if (get_binary_path(bin_name, &path) < 0) { os_free(path); return; }
    fprintf(out, "\n===== %s =====\n", title);
    fflush(out);
    wfd_t *wfd = wpopenv(path, (char **)args, W_BIND_STDOUT | W_BIND_STDERR);
    if (wfd) {
        char buf[OS_MAXSTR];
        while (fgets(buf, OS_MAXSTR, wfd->file_out)) {
            fputs(buf, out);
        }
        wpclose(wfd);
    }
    os_free(path);
}

int main(int argc, char **argv) {
    (void)argc;
    char path[OS_MAXSTR];
    char log_msg[OS_MAXSTR];
    int action = OS_INVALID;
    cJSON *input_json = NULL;

    action = setup_and_check_message(argv, &input_json);
    if ((action != ENABLE_COMMAND) && (action != DISABLE_COMMAND)) {
        return OS_INVALID;
    }
    if (action == DISABLE_COMMAND) {
        write_debug_file(argv[0], "Ended (disable is a no-op for collect-forensic)");
        cJSON_Delete(input_json);
        return OS_SUCCESS;
    }

#ifndef WIN32
    mkdir(FORENSIC_DIR, 0700);
#endif
    time_t now = time(NULL);
    memset(path, '\0', OS_MAXSTR);
    snprintf(path, OS_MAXSTR - 1, "%s/triage-%ld.txt", FORENSIC_DIR, (long)now);

    FILE *out = fopen(path, "w");
    if (!out) {
        write_debug_file(argv[0], "Cannot open forensic output file");
        cJSON_Delete(input_json);
        return OS_INVALID;
    }
    fprintf(out, "GuardSarm forensic triage @ %ld\n", (long)now);

#ifdef WIN32
    char *proc[] = { "tasklist", NULL };
    char *net[]  = { "netstat", "-ano", NULL };
    run_into("tasklist", proc, out, "processes");
    run_into("netstat", net, out, "connections");
#else
    char *proc[] = { "ps", "-ef", NULL };
    char *net[]  = { "ss", "-tunap", NULL };
    char *who[]  = { "who", NULL };
    run_into("ps", proc, out, "processes");
    run_into("ss", net, out, "connections");
    run_into("who", who, out, "logged-in users");
#endif

    fclose(out);

    memset(log_msg, '\0', OS_MAXSTR);
    snprintf(log_msg, OS_MAXSTR - 1, "Collected forensic triage into '%s'", path);
    write_debug_file(argv[0], log_msg);
    write_debug_file(argv[0], "Ended");

    cJSON_Delete(input_json);
    return OS_SUCCESS;
}
