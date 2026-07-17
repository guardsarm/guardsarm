/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying,
 * distribution, modification, or use is prohibited except under a written license
 * agreement with GuardSarm, Inc.
 */

/* GuardsArm agent tamper-protection guardian (Windows) — user-mode resilience tier.
 *
 * TWO independent SYSTEM-privileged mechanisms that guard the agent AND each other:
 *   - a scheduled task "GuardSarm Integrity Guard" (boot + logon + every minute), and
 *   - a Windows service "GuardSarmGuardianSvc" (SCM auto-restart, always running).
 * Both run the same tick. The task re-arms the service if an admin `sc delete`s it;
 * the service re-arms the task if an admin deletes it. Forked from repair-network.c's
 * fail-open watchdog pattern. It does NOT provide kernel-grade un-killability (that
 * needs a signed minifilter/OB-callback driver + PPL/ELAM); it makes every non-kernel
 * tamper attempt FAIL, self-heal, and alert. (A command wodle is deliberately NOT used
 * to drive the guardian -- this agent runs wodles de-privileged, so a wodle-invoked
 * guardian cannot perform the privileged repairs.)
 *
 * Each --auto tick (idempotent, silent unless it corrects something):
 *   1. Stand down if an MSI transaction is in progress (Global\_MSIExecute) or a
 *      disarm marker is present — so it NEVER fights a legit install/upgrade/uninstall.
 *   2. Service resilience: recreate GuardSarmSvc if deleted; force Start=auto;
 *      (re)apply SCM failure actions (auto-restart); start it if stopped/killed.
 *   3. Integrity self-heal: restore critical files from the protected backup when
 *      deleted or (for binaries) modified vs the manifest. gsmsec.conf is restored
 *      only when DELETED (admins may edit it) — modification is alert-only.
 *   4. Re-arm: re-register its own scheduled task if an attacker removed it.
 *   Every correction is written to active-responses.log as a TAMPER event, which the
 *   agent forwards to the manager (decoder guardsarm-windows-tamper -> detection).
 *
 * Modes:  --auto (watchdog tick, run by the task)   --service (run by the SCM as the
 *         guardian service)   --arm (backup+manifest+failure-actions+task+guardian
 *         service, run at install)   --disarm (drop marker + remove task + guardian
 *         service, run at uninstall)   --backup (refresh manifest only)
 *         --install-watchdog / --remove-watchdog   --status
 *
 * SAFETY: fully fail-open. A departing customer always uninstalls cleanly — the MSI
 * uninstall runs --disarm first, and any in-flight tick sees _MSIExecute / the disarm
 * marker and does nothing. No service ACL/SDDL lockdown is applied in this tier, so
 * msiexec can always stop+remove the service. */

#ifdef WIN32

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <wincrypt.h>
#ifndef CALG_SHA_256
#define CALG_SHA_256 0x0000800cU
#endif
#ifndef PROV_RSA_AES
#define PROV_RSA_AES 24
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define SVC_NAME        "GuardSarmSvc"
#define GUARD_SVC_NAME  "GuardSarmGuardianSvc"
#define GUARD_TASK      "GuardSarm Integrity Guard"
#define AGENT_EXE       "guardsarm-agent.exe"
#define LOG_REL         "\\active-response\\active-responses.log"
#define INTEG_REL       "\\.integrity"
#define MANIFEST_REL    "\\.integrity\\manifest.txt"
#define DISARM_REL      "\\.integrity\\disarm"
#define BINPATH_REL     "\\.integrity\\svc-binpath.txt"

/* Critical files (relative to install root). restore_on_modify=1 => binaries that
 * must match the manifest hash; 0 => restore only if the file is missing. */
typedef struct { const char *rel; int restore_on_modify; } critfile;
static critfile CRIT[] = {
    { AGENT_EXE,                                       1 },
    { "active-response\\bin\\guardsarm-tamper-guard.exe", 1 },
    { "gsmsec.conf",                                   0 },
};
static const int CRIT_N = (int)(sizeof(CRIT) / sizeof(CRIT[0]));

/* Resolve install root from this exe's path:
 *   <root>\active-response\bin\guardsarm-tamper-guard.exe -> <root>  */
static int install_root(char *out, size_t n) {
    char mod[MAX_PATH];
    if (GetModuleFileNameA(NULL, mod, MAX_PATH) == 0) return -1;
    for (int i = 0; i < 3; i++) {                 /* strip exe, bin, active-response */
        char *slash = strrchr(mod, '\\');
        if (!slash) return -1;
        *slash = '\0';
    }
    strncpy(out, mod, n - 1); out[n - 1] = '\0';
    return 0;
}

/* Timestamped line to the shared active-response log (agent forwards it upstream). */
static void logline(const char *root, const char *msg) {
    char path[MAX_PATH];
    snprintf(path, sizeof(path), "%s%s", root, LOG_REL);
    FILE *fp = fopen(path, "a");
    if (!fp) { printf("%s\n", msg); return; }
    time_t t = time(NULL); struct tm tmv; localtime_s(&tmv, &t);
    char ts[32]; strftime(ts, sizeof(ts), "%Y/%m/%d %H:%M:%S", &tmv);
    fprintf(fp, "%s active-response/bin/guardsarm-tamper-guard.exe: %s\n", ts, msg);
    fclose(fp); printf("%s\n", msg);
}

/* Emit a classifiable TAMPER event (kind is a stable token for the decoder/rule). */
static void tamper_event(const char *root, const char *kind, const char *detail) {
    char m[700];
    snprintf(m, sizeof(m), "TAMPER: %s - %s", kind, detail ? detail : "");
    logline(root, m);
}

/* Run a command line without a cmd.exe shell; return exit code (-1 on spawn fail). */
static int run_cmd(const char *cmdline) {
    STARTUPINFOA si; PROCESS_INFORMATION pi;
    memset(&si, 0, sizeof(si)); si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW; si.wShowWindow = SW_HIDE;
    char buf[2048]; strncpy(buf, cmdline, sizeof(buf) - 1); buf[sizeof(buf) - 1] = '\0';
    if (!CreateProcessA(NULL, buf, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) return -1;
    WaitForSingleObject(pi.hProcess, 30000);
    DWORD code = 1; GetExitCodeProcess(pi.hProcess, &code);
    CloseHandle(pi.hProcess); CloseHandle(pi.hThread);
    return (int)code;
}

/* Is an MSI install/upgrade/uninstall transaction currently running? If so the
 * guardian must stand down so it never fights a legitimate installer. */
static int msi_busy(void) {
    HANDLE h = OpenMutexA(SYNCHRONIZE, FALSE, "Global\\_MSIExecute");
    if (h) { CloseHandle(h); return 1; }
    return 0;
}

static int file_exists(const char *p) {
    DWORD a = GetFileAttributesA(p);
    return (a != INVALID_FILE_ATTRIBUTES && !(a & FILE_ATTRIBUTE_DIRECTORY));
}

/* SHA-256 of a file as lowercase hex (65-byte out buffer). 0 on success. */
static int sha256_file(const char *path, char *outhex) {
    FILE *fp = fopen(path, "rb"); if (!fp) return -1;
    HCRYPTPROV hp = 0; HCRYPTHASH hh = 0; int ok = -1;
    if (CryptAcquireContextA(&hp, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT)) {
        if (CryptCreateHash(hp, CALG_SHA_256, 0, 0, &hh)) {
            unsigned char buf[8192]; size_t n;
            while ((n = fread(buf, 1, sizeof(buf), fp)) > 0) CryptHashData(hh, buf, (DWORD)n, 0);
            BYTE dig[32]; DWORD dl = sizeof(dig);
            if (CryptGetHashParam(hh, HP_HASHVAL, dig, &dl, 0)) {
                for (DWORD i = 0; i < dl; i++) sprintf(outhex + i * 2, "%02x", dig[i]);
                outhex[dl * 2] = '\0'; ok = 0;
            }
            CryptDestroyHash(hh);
        }
        CryptReleaseContext(hp, 0);
    }
    fclose(fp);
    return ok;
}

/* Flatten a relative path into a backup filename ('\' -> '_'). */
static void flatten(const char *rel, char *out, size_t n) {
    size_t i = 0;
    for (; rel[i] && i < n - 1; i++) out[i] = (rel[i] == '\\') ? '_' : rel[i];
    out[i] = '\0';
}

static void backup_path(const char *root, const char *rel, char *out, size_t n) {
    char flat[MAX_PATH]; flatten(rel, flat, sizeof(flat));
    snprintf(out, n, "%s%s\\%s", root, INTEG_REL, flat);
}

/* Look up a stored manifest hash for rel. Returns 0 and fills hex(65) if found. */
static int manifest_hash(const char *root, const char *rel, char *hex) {
    char mpath[MAX_PATH];
    snprintf(mpath, sizeof(mpath), "%s%s", root, MANIFEST_REL);
    FILE *fp = fopen(mpath, "r"); if (!fp) return -1;
    char line[MAX_PATH + 80]; int found = -1;
    while (fgets(line, sizeof(line), fp)) {
        char *tab = strchr(line, '\t'); if (!tab) continue;
        *tab = '\0';
        if (_stricmp(line, rel) == 0) {
            char *h = tab + 1; h[strcspn(h, "\r\n")] = '\0';
            strncpy(hex, h, 64); hex[64] = '\0'; found = 0; break;
        }
    }
    fclose(fp);
    return found;
}

/* ---- install-time: snapshot critical files + write the manifest -------------- */
static int do_backup(const char *root) {
    char integ[MAX_PATH];
    snprintf(integ, sizeof(integ), "%s%s", root, INTEG_REL);
    CreateDirectoryA(integ, NULL);
    SetFileAttributesA(integ, FILE_ATTRIBUTE_HIDDEN);

    char mpath[MAX_PATH];
    snprintf(mpath, sizeof(mpath), "%s%s", root, MANIFEST_REL);
    FILE *mf = fopen(mpath, "w");
    if (!mf) { logline(root, "arm: cannot write integrity manifest"); return 1; }

    int n = 0;
    for (int i = 0; i < CRIT_N; i++) {
        char src[MAX_PATH], dst[MAX_PATH], hex[65];
        snprintf(src, sizeof(src), "%s\\%s", root, CRIT[i].rel);
        if (!file_exists(src)) continue;
        backup_path(root, CRIT[i].rel, dst, sizeof(dst));
        CopyFileA(src, dst, FALSE);
        if (sha256_file(src, hex) == 0) { fprintf(mf, "%s\t%s\n", CRIT[i].rel, hex); n++; }
    }
    fclose(mf);
    /* Capture the exact service binpath so a deleted service can be recreated faithfully. */
    SC_HANDLE scm = OpenSCManagerA(NULL, NULL, SC_MANAGER_CONNECT);
    if (scm) {
        SC_HANDLE svc = OpenServiceA(scm, SVC_NAME, SERVICE_QUERY_CONFIG);
        if (svc) {
            char qbuf[4096]; DWORD need = 0;
            if (QueryServiceConfigA(svc, (LPQUERY_SERVICE_CONFIGA)qbuf, sizeof(qbuf), &need)) {
                LPQUERY_SERVICE_CONFIGA c = (LPQUERY_SERVICE_CONFIGA)qbuf;
                char bp[MAX_PATH];
                snprintf(bp, sizeof(bp), "%s%s", root, BINPATH_REL);
                FILE *bf = fopen(bp, "w");
                if (bf) { fprintf(bf, "%s", c->lpBinaryPathName ? c->lpBinaryPathName : ""); fclose(bf); }
            }
            CloseServiceHandle(svc);
        }
        CloseServiceHandle(scm);
    }
    char m[128]; snprintf(m, sizeof(m), "arm: integrity manifest written (%d files backed up)", n);
    logline(root, m);
    return 0;
}

/* ---- service resilience ------------------------------------------------------ */
/* -1 not installed, 0 stopped, 1 running, 2 other/transitional. */
static int svc_state(const char *name) {
    SC_HANDLE scm = OpenSCManagerA(NULL, NULL, SC_MANAGER_CONNECT);
    if (!scm) return 2;
    SC_HANDLE svc = OpenServiceA(scm, name, SERVICE_QUERY_STATUS);
    int st = -1;
    if (svc) {
        SERVICE_STATUS s;
        if (QueryServiceStatus(svc, &s))
            st = (s.dwCurrentState == SERVICE_RUNNING) ? 1 :
                 (s.dwCurrentState == SERVICE_STOPPED) ? 0 : 2;
        else st = 2;
        CloseServiceHandle(svc);
    }
    CloseServiceHandle(scm);
    return st;
}
static int service_state(void) { return svc_state(SVC_NAME); }

static void read_binpath(const char *root, char *out, size_t n) {
    char bp[MAX_PATH];
    snprintf(bp, sizeof(bp), "%s%s", root, BINPATH_REL);
    FILE *fp = fopen(bp, "r");
    if (fp) {
        if (fgets(out, (int)n, fp)) { out[strcspn(out, "\r\n")] = '\0'; fclose(fp); if (out[0]) return; }
        else fclose(fp);
    }
    snprintf(out, n, "\"%s\\%s\"", root, AGENT_EXE);   /* fallback */
}

/* Ensure the service exists, is auto-start, has SCM restart actions, and is running.
 * Returns the number of corrective actions taken. */
static int ensure_service(const char *root) {
    int fixes = 0;
    int st = service_state();

    if (st == -1) {                                  /* attacker ran `sc delete` */
        char binpath[MAX_PATH], cmd[MAX_PATH + 160];
        read_binpath(root, binpath, sizeof(binpath));
        snprintf(cmd, sizeof(cmd),
                 "sc create %s binPath= %s start= auto DisplayName= \"GuardSarm\"",
                 SVC_NAME, binpath);
        if (run_cmd(cmd) == 0) { tamper_event(root, "service_deleted", "recreated GuardSarmSvc"); fixes++; st = 0; }
        else { tamper_event(root, "service_deleted", "recreate FAILED"); return fixes; }
    }

    /* Idempotently enforce auto-start + failure actions (cheap; also repairs an
     * attacker who set start=disabled or cleared the recovery actions). */
    run_cmd("sc config " SVC_NAME " start= auto");
    run_cmd("sc failure " SVC_NAME " reset= 86400 actions= restart/5000/restart/5000/restart/60000");

    if (st != 1) {                                   /* stopped/killed -> restart */
        if (run_cmd("sc start " SVC_NAME) == 0) { tamper_event(root, "service_stopped", "restarted GuardSarmSvc"); fixes++; }
    }
    return fixes;
}

/* ---- integrity self-heal ----------------------------------------------------- */
static int restore_from_backup(const char *root, const char *rel) {
    char dst[MAX_PATH], bak[MAX_PATH];
    snprintf(dst, sizeof(dst), "%s\\%s", root, rel);
    backup_path(root, rel, bak, sizeof(bak));
    if (!file_exists(bak)) return -1;                /* no backup -> cannot heal */
    return CopyFileA(bak, dst, FALSE) ? 0 : -1;
}

static int ensure_integrity(const char *root) {
    int fixes = 0;
    for (int i = 0; i < CRIT_N; i++) {
        char full[MAX_PATH];
        snprintf(full, sizeof(full), "%s\\%s", root, CRIT[i].rel);

        char det[MAX_PATH + 48];
        if (!file_exists(full)) {                    /* deleted */
            if (restore_from_backup(root, CRIT[i].rel) == 0) {
                snprintf(det, sizeof(det), "%s (restored from backup)", CRIT[i].rel);
                tamper_event(root, "file_deleted", det); fixes++;
            } else {
                snprintf(det, sizeof(det), "%s (unrecoverable: no backup)", CRIT[i].rel);
                tamper_event(root, "file_deleted", det);
            }
            continue;
        }
        if (CRIT[i].restore_on_modify) {             /* binaries: verify hash */
            char want[65], have[65];
            if (manifest_hash(root, CRIT[i].rel, want) == 0 && sha256_file(full, have) == 0
                && _stricmp(want, have) != 0) {
                if (restore_from_backup(root, CRIT[i].rel) == 0) {
                    snprintf(det, sizeof(det), "%s (restored from backup)", CRIT[i].rel);
                    tamper_event(root, "file_modified", det); fixes++;
                } else {
                    snprintf(det, sizeof(det), "%s (unrecoverable: no backup)", CRIT[i].rel);
                    tamper_event(root, "file_modified", det);
                }
            }
        } else {                                     /* config: alert-only on change */
            char want[65], have[65];
            if (manifest_hash(root, CRIT[i].rel, want) == 0 && sha256_file(full, have) == 0
                && _stricmp(want, have) != 0)
                tamper_event(root, "config_changed", CRIT[i].rel);
        }
    }
    return fixes;
}

/* ---- guardian scheduled task (self-contained; no PowerShell) ----------------- */
static int write_task_xml(const char *xmlpath, const char *self_exe) {
    char xml[3072];
    snprintf(xml, sizeof(xml),
        "<?xml version=\"1.0\" encoding=\"UTF-16\"?>\n"
        "<Task version=\"1.2\" xmlns=\"http://schemas.microsoft.com/windows/2004/02/mit/task\">\n"
        "  <RegistrationInfo><Description>GuardSarm agent integrity guardian. Keeps the agent service alive and its binaries/config intact against tampering. Fail-open: stands down during any MSI transaction.</Description></RegistrationInfo>\n"
        "  <Triggers>\n"
        "    <BootTrigger><Enabled>true</Enabled></BootTrigger>\n"
        "    <LogonTrigger><Enabled>true</Enabled></LogonTrigger>\n"
        "    <TimeTrigger><StartBoundary>2020-01-01T00:00:00</StartBoundary><Enabled>true</Enabled><Repetition><Interval>PT1M</Interval><StopAtDurationEnd>false</StopAtDurationEnd></Repetition></TimeTrigger>\n"
        "  </Triggers>\n"
        "  <Principals><Principal id=\"Author\"><UserId>S-1-5-18</UserId><RunLevel>HighestAvailable</RunLevel></Principal></Principals>\n"
        "  <Settings><MultipleInstancesPolicy>IgnoreNew</MultipleInstancesPolicy><DisallowStartIfOnBatteries>false</DisallowStartIfOnBatteries><StopIfGoingOnBatteries>false</StopIfGoingOnBatteries><StartWhenAvailable>true</StartWhenAvailable><ExecutionTimeLimit>PT2M</ExecutionTimeLimit><Enabled>true</Enabled></Settings>\n"
        "  <Actions Context=\"Author\"><Exec><Command>%s</Command><Arguments>--auto</Arguments></Exec></Actions>\n"
        "</Task>\n", self_exe);
    FILE *fp = fopen(xmlpath, "wb");
    if (!fp) return -1;
    unsigned char bom[2] = { 0xFF, 0xFE }; fwrite(bom, 1, 2, fp);
    for (const char *p = xml; *p; p++) { fputc((unsigned char)*p, fp); fputc(0x00, fp); }
    fclose(fp);
    return 0;
}

static int install_watchdog(const char *root, const char *self_exe) {
    char xmlpath[MAX_PATH], cmd[MAX_PATH + 256];
    snprintf(xmlpath, sizeof(xmlpath), "%s\\active-response\\gsm-integrity-task.xml", root);
    if (write_task_xml(xmlpath, self_exe) != 0) { logline(root, "install-watchdog: cannot write task XML"); return 1; }
    snprintf(cmd, sizeof(cmd), "schtasks /Create /TN \"%s\" /XML \"%s\" /F", GUARD_TASK, xmlpath);
    int rc = run_cmd(cmd);
    DeleteFileA(xmlpath);
    logline(root, rc == 0 ? "install-watchdog: registered '" GUARD_TASK "' (boot+logon+1min, SYSTEM)"
                          : "install-watchdog: schtasks /Create failed");
    return rc == 0 ? 0 : 1;
}

static int remove_watchdog(const char *root) {
    run_cmd("schtasks /Delete /TN \"" GUARD_TASK "\" /F");
    logline(root, "remove-watchdog: task removed (or absent)");
    return 0;
}

static int task_exists(void) {
    return run_cmd("schtasks /Query /TN \"" GUARD_TASK "\"") == 0;
}

/* ---- disarm marker (uninstall guarantee) ------------------------------------- */
static int disarmed(const char *root) {
    char p[MAX_PATH]; snprintf(p, sizeof(p), "%s%s", root, DISARM_REL);
    return file_exists(p);
}
static void set_disarm(const char *root, int on) {
    char p[MAX_PATH]; snprintf(p, sizeof(p), "%s%s", root, DISARM_REL);
    if (on) { FILE *f = fopen(p, "w"); if (f) { fprintf(f, "disarmed %lld\n", (long long)time(NULL)); fclose(f); } }
    else    { DeleteFileA(p); }
}

/* ---- guardian Windows service (privileged; mutual re-arm with the task) -------- */
/* Create (if deleted) + auto-start + restart-actions the guardian service. The command
 * wodle path is dead (this agent runs wodles de-privileged), so this SYSTEM service is
 * what re-arms the scheduled task if an admin deletes it -- and the task re-arms this
 * service if it is deleted. Two independent privileged persistence mechanisms guarding
 * each other. Returns 1 if it corrected something. */
static int ensure_guardian_service(const char *root, const char *self) {
    int st = svc_state(GUARD_SVC_NAME);
    if (st == -1) {                                  /* attacker ran `sc delete` on us */
        /* Use the 8.3 short path so the binPath has no spaces -- sc create's quoting of a
         * "C:\Program Files (x86)\..." path plus an argument is famously fragile; a short
         * path sidesteps it entirely. Fall back to the long path if 8.3 is unavailable. */
        char shortself[MAX_PATH];
        if (GetShortPathNameA(self, shortself, MAX_PATH) == 0) {
            strncpy(shortself, self, MAX_PATH - 1); shortself[MAX_PATH - 1] = '\0';
        }
        char cmd[MAX_PATH + 220];
        snprintf(cmd, sizeof(cmd),
                 "sc create %s binPath= \"%s --service\" start= auto DisplayName= \"GuardSarm Integrity Service\"",
                 GUARD_SVC_NAME, shortself);
        run_cmd(cmd);
        run_cmd("sc failure " GUARD_SVC_NAME " reset= 86400 actions= restart/5000/restart/5000/restart/60000");
        run_cmd("sc start " GUARD_SVC_NAME);
        tamper_event(root, "guardian_service_deleted", "recreated " GUARD_SVC_NAME);
        return 1;
    }
    run_cmd("sc config " GUARD_SVC_NAME " start= auto");
    run_cmd("sc failure " GUARD_SVC_NAME " reset= 86400 actions= restart/5000/restart/5000/restart/60000");
    if (st != 1) { run_cmd("sc start " GUARD_SVC_NAME); return 1; }
    return 0;
}

static void remove_guardian_service(void) {
    run_cmd("sc stop " GUARD_SVC_NAME);
    run_cmd("sc delete " GUARD_SVC_NAME);
}

/* The shared guardian tick, run by BOTH the scheduled task (--auto) and the guardian
 * service loop. Ensures: the agent service, file integrity, the scheduled task, and the
 * guardian service. The task and the service therefore re-arm each other. */
static int guardian_tick(const char *root, const char *self) {
    int fixes = 0;
    fixes += ensure_service(root);
    fixes += ensure_integrity(root);
    if (!task_exists()) {
        if (install_watchdog(root, self) == 0) { tamper_event(root, "task_removed", "re-armed '" GUARD_TASK "'"); fixes++; }
        else tamper_event(root, "task_removed", "re-arm FAILED (insufficient privilege?) for '" GUARD_TASK "'");
    }
    fixes += ensure_guardian_service(root, self);
    return fixes;
}

/* Windows service scaffolding for `--service`. */
static SERVICE_STATUS        g_svc_status;
static SERVICE_STATUS_HANDLE g_svc_handle;
static HANDLE                g_svc_stop;

static void WINAPI svc_ctrl(DWORD ctrl) {
    if (ctrl == SERVICE_CONTROL_STOP || ctrl == SERVICE_CONTROL_SHUTDOWN) {
        g_svc_status.dwCurrentState = SERVICE_STOP_PENDING;
        SetServiceStatus(g_svc_handle, &g_svc_status);
        if (g_svc_stop) SetEvent(g_svc_stop);
    }
}

static void WINAPI svc_main(DWORD argc, LPSTR *argv) {
    (void)argc; (void)argv;
    g_svc_handle = RegisterServiceCtrlHandlerA(GUARD_SVC_NAME, svc_ctrl);
    if (!g_svc_handle) return;
    memset(&g_svc_status, 0, sizeof(g_svc_status));
    g_svc_status.dwServiceType  = SERVICE_WIN32_OWN_PROCESS;
    g_svc_status.dwCurrentState = SERVICE_START_PENDING;
    SetServiceStatus(g_svc_handle, &g_svc_status);

    g_svc_stop = CreateEvent(NULL, TRUE, FALSE, NULL);
    g_svc_status.dwCurrentState     = SERVICE_RUNNING;
    g_svc_status.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
    SetServiceStatus(g_svc_handle, &g_svc_status);

    char root[MAX_PATH], self[MAX_PATH];
    if (install_root(root, sizeof(root)) != 0) strcpy(root, ".");
    GetModuleFileNameA(NULL, self, MAX_PATH);

    do {
        if (!msi_busy() && !disarmed(root)) guardian_tick(root, self);
    } while (WaitForSingleObject(g_svc_stop, 45000) != WAIT_OBJECT_0);

    g_svc_status.dwCurrentState = SERVICE_STOPPED;
    SetServiceStatus(g_svc_handle, &g_svc_status);
}

static int run_as_service(void) {
    SERVICE_TABLE_ENTRYA tbl[] = { { (LPSTR)GUARD_SVC_NAME, svc_main }, { NULL, NULL } };
    return StartServiceCtrlDispatcherA(tbl) ? 0 : 1;
}

int main(int argc, char **argv) {
    int m_auto = 0, m_arm = 0, m_disarm = 0, m_backup = 0, m_inst = 0, m_rem = 0, m_status = 0, m_service = 0;
    for (int i = 1; i < argc; i++) {
        if      (!strcmp(argv[i], "--auto"))             m_auto = 1;
        else if (!strcmp(argv[i], "--arm"))              m_arm = 1;
        else if (!strcmp(argv[i], "--disarm"))           m_disarm = 1;
        else if (!strcmp(argv[i], "--backup"))           m_backup = 1;
        else if (!strcmp(argv[i], "--install-watchdog")) m_inst = 1;
        else if (!strcmp(argv[i], "--remove-watchdog"))  m_rem = 1;
        else if (!strcmp(argv[i], "--status"))           m_status = 1;
        else if (!strcmp(argv[i], "--service"))          m_service = 1;
    }

    char root[MAX_PATH], self[MAX_PATH];
    if (install_root(root, sizeof(root)) != 0) strcpy(root, ".");
    GetModuleFileNameA(NULL, self, MAX_PATH);

    /* --service: launched by the SCM. Hands control to the service dispatcher (blocks). */
    if (m_service) return run_as_service();

    /* --arm: run at install/upgrade. backup + manifest + failure actions + scheduled task
     * + guardian service, and CLEAR any stale disarm marker so a reinstall is protected. */
    if (m_arm) {
        set_disarm(root, 0);
        do_backup(root);
        run_cmd("sc failure " SVC_NAME " reset= 86400 actions= restart/5000/restart/5000/restart/60000");
        install_watchdog(root, self);
        ensure_guardian_service(root, self);
        return 0;
    }
    /* --disarm: run FIRST on uninstall. Marker + task + guardian-service removal guarantee
     * a clean exit (also releases the guardian exe so RemoveFiles can delete it). */
    if (m_disarm) {
        set_disarm(root, 1);
        remove_watchdog(root);
        remove_guardian_service();
        logline(root, "disarm: integrity guardian disarmed for uninstall (task + service removed)");
        return 0;
    }
    if (m_backup)  return do_backup(root);
    if (m_inst)    return install_watchdog(root, self);
    if (m_rem)     return remove_watchdog(root);

    if (m_status) {
        char msg[320];
        snprintf(msg, sizeof(msg), "status: agent_service=%s task=%s guardian_service=%s disarmed=%s",
                 service_state() == 1 ? "running" : (service_state() == -1 ? "not-installed" : "stopped"),
                 task_exists() ? "present" : "absent",
                 svc_state(GUARD_SVC_NAME) == 1 ? "running" : (svc_state(GUARD_SVC_NAME) == -1 ? "not-installed" : "stopped"),
                 disarmed(root) ? "YES" : "no");
        logline(root, msg);
        return 0;
    }

    /* ---- default: watchdog tick (--auto, run by the scheduled task) ----------- */
    /* Stand down during any MSI transaction or once disarmed for uninstall. */
    if (msi_busy())     return 0;
    if (disarmed(root)) return 0;

    int fixes = guardian_tick(root, self);
    return fixes > 0 ? 10 : 0;   /* 10 = took corrective action (informational) */
}

#else /* !WIN32 */
int main(void) { return 0; }
#endif
