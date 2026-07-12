/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying,
 * distribution, modification, or use is prohibited except under a written license
 * agreement with GuardSarm, Inc.
 */

/* GuardsArm network repair / isolation recovery utility (Windows).
 *
 * Two roles, one binary:
 *   1. Emergency operator tool:  repair-network.exe --force
 *        Unconditionally removes every GuardsArm WFP filter/sub-layer/provider,
 *        flushes DNS, and verifies connectivity. Use if an endpoint is ever stuck.
 *   2. Recovery watchdog (scheduled task): repair-network.exe --auto
 *        Fails OPEN — removes isolation ONLY when it must not persist:
 *          - isolation present but no/stale state file (orphaned), OR
 *          - isolation older than its dead-man timeout (default 30 min), OR
 *          - isolation present but the GuardSarmSvc agent service is not running
 *            (agent crashed or was uninstalled while isolated — the exact incident
 *            this design exists to prevent).
 *        A healthy, in-window isolation is left untouched.
 *
 * Other modes:  --status (report only, no change)   --reset-winsock (with --force)
 *
 * This tool NEVER modifies global firewall policy, routes, or adapter bindings —
 * it only removes GuardsArm's own WFP objects. Safe by construction. */

#ifdef WIN32

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "helpers/wfp_isolation.h"

#define SVC_NAME        "GuardSarmSvc"
#define STATE_REL       "\\active-response\\isolation.json"
#define LOG_REL         "\\active-response\\active-responses.log"
#define DEFAULT_TIMEOUT 1800

/* Resolve the agent install root from this exe's path:
 *   <root>\active-response\bin\repair-network.exe  ->  <root>  */
static int install_root(char *out, size_t n) {
    char mod[MAX_PATH];
    if (GetModuleFileNameA(NULL, mod, MAX_PATH) == 0) return -1;
    /* strip three path components: repair-network.exe, bin, active-response */
    for (int i = 0; i < 3; i++) {
        char *slash = strrchr(mod, '\\');
        if (!slash) return -1;
        *slash = '\0';
    }
    strncpy(out, mod, n - 1);
    out[n - 1] = '\0';
    return 0;
}

/* Append a timestamped line to the shared active-response log (best-effort). */
static void logline(const char *root, const char *msg) {
    char path[MAX_PATH];
    snprintf(path, sizeof(path), "%s%s", root, LOG_REL);
    FILE *fp = fopen(path, "a");
    if (!fp) { printf("%s\n", msg); return; }
    time_t t = time(NULL);
    struct tm tmv; localtime_s(&tmv, &t);
    char ts[32]; strftime(ts, sizeof(ts), "%Y/%m/%d %H:%M:%S", &tmv);
    fprintf(fp, "%s active-response/bin/repair-network.exe: %s\n", ts, msg);
    fclose(fp);
    printf("%s\n", msg);
}

/* Read the isolation state file. Returns 1 if a valid "active":true state was read
 * (epoch/timeout filled), 0 if not present / not active. */
static int read_state(const char *root, long long *epoch, unsigned *timeout) {
    char path[MAX_PATH];
    snprintf(path, sizeof(path), "%s%s", root, STATE_REL);
    FILE *fp = fopen(path, "r");
    if (!fp) return 0;
    char buf[512] = {0};
    size_t rd = fread(buf, 1, sizeof(buf) - 1, fp);
    fclose(fp);
    buf[rd] = '\0';
    if (!strstr(buf, "\"active\":true")) return 0;
    char *pe = strstr(buf, "\"epoch\":");
    char *pt = strstr(buf, "\"timeout\":");
    *epoch   = pe ? _atoi64(pe + 8) : 0;
    *timeout = pt ? (unsigned)atoi(pt + 10) : DEFAULT_TIMEOUT;
    return 1;
}

/* Is the agent service currently RUNNING? -1 = service not installed. */
static int service_running(void) {
    SC_HANDLE scm = OpenSCManagerA(NULL, NULL, SC_MANAGER_CONNECT);
    if (!scm) return 0;
    SC_HANDLE svc = OpenServiceA(scm, SVC_NAME, SERVICE_QUERY_STATUS);
    if (!svc) { CloseServiceHandle(scm); return -1; }
    SERVICE_STATUS st; int running = 0;
    if (QueryServiceStatus(svc, &st)) running = (st.dwCurrentState == SERVICE_RUNNING);
    CloseServiceHandle(svc);
    CloseServiceHandle(scm);
    return running;
}

static void clear_state(const char *root) {
    char path[MAX_PATH];
    snprintf(path, sizeof(path), "%s%s", root, STATE_REL);
    FILE *fp = fopen(path, "w");
    if (fp) { fprintf(fp, "{\"active\":false,\"epoch\":%lld}\n", (long long)time(NULL)); fclose(fp); }
}

int main(int argc, char **argv) {
    int mode_auto = 0, mode_force = 0, mode_status = 0, reset_winsock = 0;
    for (int i = 1; i < argc; i++) {
        if      (!strcmp(argv[i], "--auto"))          mode_auto = 1;
        else if (!strcmp(argv[i], "--force"))         mode_force = 1;
        else if (!strcmp(argv[i], "--status"))        mode_status = 1;
        else if (!strcmp(argv[i], "--reset-winsock")) reset_winsock = 1;
    }
    if (!mode_auto && !mode_force && !mode_status) mode_force = 1;  /* default: repair */

    char root[MAX_PATH];
    if (install_root(root, sizeof(root)) != 0) strcpy(root, ".");

    int isolated = wfp_is_isolated();
    long long epoch = 0; unsigned timeout = DEFAULT_TIMEOUT;
    int have_state = read_state(root, &epoch, &timeout);
    int svc = service_running();
    long long age = have_state ? ((long long)time(NULL) - epoch) : -1;

    /* ---- status: report and exit ---------------------------------------- */
    if (mode_status) {
        char m[512];
        snprintf(m, sizeof(m),
            "status: isolated=%s state=%s age=%llds timeout=%us service=%s",
            isolated ? "YES" : "no",
            have_state ? "active" : "none",
            (long long)age, timeout,
            svc == 1 ? "running" : (svc == -1 ? "not-installed" : "stopped"));
        logline(root, m);
        return isolated ? 10 : 0;
    }

    /* ---- auto (watchdog): fail open only when required ------------------- */
    if (mode_auto) {
        if (!isolated) { logline(root, "auto: not isolated — nothing to do"); return 0; }
        const char *why = NULL;
        if (!have_state)                       why = "orphaned isolation (no active state file)";
        else if (age > (long long)timeout)     why = "dead-man timeout exceeded";
        else if (svc == -1)                    why = "agent service not installed (uninstalled while isolated)";
        else if (svc == 0)                     why = "agent service not running (crashed while isolated)";
        if (!why) { logline(root, "auto: isolation healthy and in-window — leaving intact"); return 0; }

        char m[600]; snprintf(m, sizeof(m), "auto: FAIL-OPEN triggered — %s; restoring networking", why);
        logline(root, m);
    }

    /* ---- force / auto-triggered: full repair ---------------------------- */
    wfp_report rep;
    int r = wfp_repair_network(reset_winsock ? true : false, &rep);
    clear_state(root);
    logline(root, rep.detail);
    logline(root, r == 0 ? "repair-network: COMPLETE (networking restored)"
                         : "repair-network: engine error during repair (see detail)");
    return r == 0 ? 0 : 1;
}

#else /* !WIN32 */
int main(void) { return 0; }
#endif
