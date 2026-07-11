/* Copyright (C) 2026 GuardSarm, Inc.
 * All right reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

/* Active response: block-domain — sinkhole a domain via the OS hosts file
 * (0.0.0.0 <domain>). Message carries a top-level "domain".
 * ENABLE adds the sinkhole entry; DISABLE removes it. Idempotent. */

#include "active_responses.h"

#ifdef WIN32
#define HOSTS_FILE "C:\\Windows\\System32\\drivers\\etc\\hosts"
#else
#define HOSTS_FILE "/etc/hosts"
#endif

#define AR_TAG " # GuardSarm-AR"

int main(int argc, char **argv) {
    (void)argc;
    char log_msg[OS_MAXSTR];
    char line[OS_MAXSTR];
    int action = OS_INVALID;
    cJSON *input_json = NULL;

    action = setup_and_check_message(argv, &input_json);
    if ((action != ENABLE_COMMAND) && (action != DISABLE_COMMAND)) {
        return OS_INVALID;
    }

    const cJSON *dom_json = cJSON_GetObjectItem(input_json, "domain");
    const char *domain = (dom_json && cJSON_IsString(dom_json)) ? dom_json->valuestring : NULL;
    if (!domain || !*domain || strpbrk(domain, " \t\r\n")) {
        write_debug_file(argv[0], "Cannot read a valid 'domain' from data");
        cJSON_Delete(input_json);
        return OS_INVALID;
    }

    if (action == ENABLE_COMMAND) {
        // Skip if already present.
        FILE *rf = fopen(HOSTS_FILE, "r");
        if (rf) {
            while (fgets(line, OS_MAXSTR, rf)) {
                if (strstr(line, domain) && strstr(line, AR_TAG)) {
                    fclose(rf);
                    write_debug_file(argv[0], "Domain already blocked; Ended");
                    cJSON_Delete(input_json);
                    return OS_SUCCESS;
                }
            }
            fclose(rf);
        }
        FILE *f = fopen(HOSTS_FILE, "a");
        if (!f) {
            memset(log_msg, '\0', OS_MAXSTR);
            snprintf(log_msg, OS_MAXSTR - 1, "Cannot open hosts file: %s", strerror(errno));
            write_debug_file(argv[0], log_msg);
            cJSON_Delete(input_json);
            return OS_INVALID;
        }
        fprintf(f, "0.0.0.0 %s%s\n", domain, AR_TAG);
        fprintf(f, "0.0.0.0 www.%s%s\n", domain, AR_TAG);
        fclose(f);
        memset(log_msg, '\0', OS_MAXSTR);
        snprintf(log_msg, OS_MAXSTR - 1, "Sinkholed domain '%s' in hosts file", domain);
        write_debug_file(argv[0], log_msg);
    } else {
        // DISABLE: rewrite the hosts file without our AR lines for this domain.
        FILE *rf = fopen(HOSTS_FILE, "r");
        if (!rf) {
            write_debug_file(argv[0], "Cannot open hosts file to unblock");
            cJSON_Delete(input_json);
            return OS_INVALID;
        }
        char tmp[OS_MAXSTR];
        snprintf(tmp, OS_MAXSTR - 1, "%s.gsar.tmp", HOSTS_FILE);
        FILE *wf = fopen(tmp, "w");
        if (!wf) { fclose(rf); write_debug_file(argv[0], "Cannot open temp hosts file"); cJSON_Delete(input_json); return OS_INVALID; }
        while (fgets(line, OS_MAXSTR, rf)) {
            if (strstr(line, domain) && strstr(line, AR_TAG)) continue; // drop our block line
            fputs(line, wf);
        }
        fclose(rf);
        fclose(wf);
        if (rename(tmp, HOSTS_FILE) != 0) {
            write_debug_file(argv[0], "Failed to replace hosts file");
            cJSON_Delete(input_json);
            return OS_INVALID;
        }
        memset(log_msg, '\0', OS_MAXSTR);
        snprintf(log_msg, OS_MAXSTR - 1, "Removed sinkhole for domain '%s'", domain);
        write_debug_file(argv[0], log_msg);
    }

    write_debug_file(argv[0], "Ended");
    cJSON_Delete(input_json);
    return OS_SUCCESS;
}
