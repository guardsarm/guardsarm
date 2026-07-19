/* Copyright (C) 2026 GuardSarm, Inc.
 * All right reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

/* Active response: block-hash — add a file hash to the agent's local denylist
 * (active-response/blocked-hashes.list). Message carries a top-level "hash".
 * ENABLE adds; DISABLE removes. The EDR/scan layer consults this denylist to
 * quarantine matching files — the AR itself maintains the authoritative list. */

#include "active_responses.h"

#define BLOCKLIST "blocked-hashes.list"

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

    const cJSON *hash_json = cJSON_GetObjectItem(input_json, "hash");
    const char *hash = (hash_json && cJSON_IsString(hash_json)) ? hash_json->valuestring : NULL;
    if (!hash || strlen(hash) < 8 || strpbrk(hash, " \t\r\n/\\")) {
        write_debug_file(argv[0], "Cannot read a valid 'hash' from data");
        cJSON_Delete(input_json);
        return OS_INVALID;
    }

    if (action == ENABLE_COMMAND) {
        FILE *rf = fopen(BLOCKLIST, "r");
        if (rf) {
            while (fgets(line, OS_MAXSTR, rf)) {
                line[strcspn(line, "\r\n")] = '\0';
                if (!strcasecmp(line, hash)) {
                    fclose(rf);
                    write_debug_file(argv[0], "Hash already blocked; Ended");
                    cJSON_Delete(input_json);
                    return OS_SUCCESS;
                }
            }
            fclose(rf);
        }
        FILE *f = fopen(BLOCKLIST, "a");
        if (!f) { write_debug_file(argv[0], "Cannot open hash denylist"); cJSON_Delete(input_json); return OS_INVALID; }
        fprintf(f, "%s\n", hash);
        fclose(f);
        memset(log_msg, '\0', OS_MAXSTR);
        snprintf(log_msg, OS_MAXSTR - 1, "Added hash '%s' to the denylist", hash);
        write_debug_file(argv[0], log_msg);
    } else {
        FILE *rf = fopen(BLOCKLIST, "r");
        if (!rf) { write_debug_file(argv[0], "No denylist to modify; Ended"); cJSON_Delete(input_json); return OS_SUCCESS; }
        char tmp[OS_MAXSTR];
        snprintf(tmp, OS_MAXSTR - 1, "%s.tmp", BLOCKLIST);
        FILE *wf = fopen(tmp, "w");
        if (!wf) { fclose(rf); write_debug_file(argv[0], "Cannot open temp denylist"); cJSON_Delete(input_json); return OS_INVALID; }
        while (fgets(line, OS_MAXSTR, rf)) {
            char cmp[OS_MAXSTR]; snprintf(cmp, OS_MAXSTR, "%s", line); cmp[strcspn(cmp, "\r\n")] = '\0';
            if (!strcasecmp(cmp, hash)) continue;
            fputs(line, wf);
        }
        fclose(rf); fclose(wf);
        // Honor the rename result: if the denylist couldn't be replaced the hash was NOT
        // removed -- report failure rather than a false "removed" (matches block-domain).
        if (rename(tmp, BLOCKLIST) != 0) {
            write_debug_file(argv[0], "Failed to update the hash denylist (rename)");
            cJSON_Delete(input_json);
            return OS_INVALID;
        }
        memset(log_msg, '\0', OS_MAXSTR);
        snprintf(log_msg, OS_MAXSTR - 1, "Removed hash '%s' from the denylist", hash);
        write_debug_file(argv[0], log_msg);
    }

    write_debug_file(argv[0], "Ended");
    cJSON_Delete(input_json);
    return OS_SUCCESS;
}
