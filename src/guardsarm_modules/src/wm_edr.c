/*
 * GuardSarm Module for native EDR telemetry (process + network)
 * Copyright (C) 2026 GuardSarm, Inc.
 *
 * See wm_edr.h for a description. Native, self-contained: no external library,
 * no dbsync — a lightweight /proc sweeper that streams JSON telemetry to the
 * manager through the standard agent queue (and mirrors it to a local log the
 * host applier already ingests).
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef WIN32   // Linux/Unix agent module (native /proc telemetry).

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <pwd.h>
#include <unistd.h>
#include <time.h>
#include <limits.h>

#include "wmodules.h"
#include "wm_edr.h"
#include "defs.h"
#include "mq_op.h"
#include "cJSON.h"
#include "sha256_op.h"

static void* wm_edr_main(wm_edr_t *edr);           // Module main function. It won't return
static void wm_edr_destroy(wm_edr_t *edr);         // Destroy configuration
static cJSON *wm_edr_dump(const wm_edr_t *edr);    // Dump current configuration

const wm_context WM_EDR_CONTEXT = {
    .name = WM_EDR_CONTEXT_NAME,
    .start = (wm_routine)wm_edr_main,
    .destroy = (void(*)(void *))wm_edr_destroy,
    .dump = (cJSON *(*)(const void *))wm_edr_dump,
    .sync = NULL,
    .stop = NULL,
    .query = NULL,
};

static int queue_fd = 0;                           // Output queue file descriptor
static OSHash *seen_procs = NULL;                  // Seen "pid:starttime" -> emitted
static OSHash *seen_conns = NULL;                  // Seen "l-r-state" -> emitted
static OSHash *seen_persist = NULL;                // Seen persistence path -> emitted
static FILE *edr_log = NULL;                       // Mirror log for the host applier

/* ------------------------------------------------------------------ helpers */

// Convert a /proc/net little-endian hex address (e.g. "0100007F") to dotted IPv4.
static void hex_to_ipv4(const char *hex, char *out, size_t out_sz) {
    unsigned int a = 0, b = 0, c = 0, d = 0;
    if (sscanf(hex, "%2x%2x%2x%2x", &d, &c, &b, &a) == 4) {
        snprintf(out, out_sz, "%u.%u.%u.%u", a, b, c, d);
    } else {
        snprintf(out, out_sz, "0.0.0.0");
    }
}

static const char *tcp_state(unsigned int st) {
    switch (st) {
        case 0x01: return "established";
        case 0x02: return "syn_sent";
        case 0x0A: return "listening";
        case 0x06: return "time_wait";
        case 0x08: return "close_wait";
        default:   return "other";
    }
}

// Send one JSON event to the manager queue AND mirror it to the applier log.
static void wm_edr_emit(wm_edr_t *edr, cJSON *event) {
    char *msg = cJSON_PrintUnformatted(event);
    if (!msg) {
        return;
    }
    const int eps = edr->max_eps > 0 ? (1000000 / (int)edr->max_eps) : 0;
    if (wm_sendmsg(eps, queue_fd, msg, WM_EDR_CONTEXT_NAME, LOCALFILE_MQ) < 0) {
        mterror(WM_EDR_LOGTAG, "Unable to send message to '%s'", DEFAULTQUEUE);
        if ((queue_fd = StartMQ(DEFAULTQUEUE, WRITE, 1)) >= 0) {
            wm_sendmsg(eps, queue_fd, msg, WM_EDR_CONTEXT_NAME, LOCALFILE_MQ);
        }
    }
    if (edr_log) {
        fprintf(edr_log, "%s\n", msg);
        fflush(edr_log);
    }
    free(msg);
}

// Resolve a numeric uid (from /proc/<pid>/status) to a user name.
static void proc_user(const char *pid, char *out, size_t out_sz) {
    char path[256];
    snprintf(path, sizeof(path), "/proc/%s/status", pid);
    FILE *f = fopen(path, "r");
    out[0] = '\0';
    if (!f) {
        return;
    }
    char line[256];
    while (fgets(line, sizeof(line), f)) {
        unsigned int uid;
        if (sscanf(line, "Uid:\t%u", &uid) == 1) {
            struct passwd *pw = getpwuid(uid);
            if (pw && pw->pw_name) {
                snprintf(out, out_sz, "%s", pw->pw_name);
            } else {
                snprintf(out, out_sz, "%u", uid);
            }
            break;
        }
    }
    fclose(f);
}

// Known script interpreters — a process whose executable basename matches is a
// script-execution event (MITRE T1059).
static const char *SCRIPT_INTERPRETERS[] = {
    "bash", "sh", "dash", "zsh", "ksh", "csh", "tcsh", "ash",
    "python", "python2", "python3", "perl", "ruby", "php", "node", "lua",
    "powershell", "pwsh", "wscript", "cscript", "cmd", NULL};

static int basename_is_interpreter(const char *name) {
    for (int i = 0; SCRIPT_INTERPRETERS[i]; i++) {
        if (!strcmp(name, SCRIPT_INTERPRETERS[i])) return 1;
    }
    return 0;
}

// Heuristic: encoded / download-and-execute command-line patterns (T1059/T1105).
static int cmd_is_suspicious(const char *cmd) {
    if (!cmd || !cmd[0]) return 0;
    static const char *SIG[] = {
        "base64 -d", "base64 --decode", "-enc", "-EncodedCommand", "FromBase64String",
        "curl ", "wget ", "|sh", "| sh", "|bash", "| bash", "IEX(", "Invoke-Expression",
        "/dev/tcp/", "nc -e", "ncat -e", "chmod +x", "eval ", NULL};
    for (int i = 0; SIG[i]; i++) {
        if (strstr(cmd, SIG[i])) return 1;
    }
    return 0;
}

// Emit a process-exec event for a newly-seen PID.
static void emit_process(wm_edr_t *edr, const char *pid) {
    char path[256], comm[256] = "", cmdline[4096] = "", user[64] = "";
    long ppid = 0;
    unsigned long long starttime = 0;

    // /proc/<pid>/stat: "pid (comm) state ppid ... starttime(22) ..."
    snprintf(path, sizeof(path), "/proc/%s/stat", pid);
    FILE *f = fopen(path, "r");
    if (!f) {
        return;
    }
    char buf[8192];
    size_t n = fread(buf, 1, sizeof(buf) - 1, f);
    fclose(f);
    if (n == 0) {
        return;
    }
    buf[n] = '\0';
    char *rp = strrchr(buf, ')');            // comm may contain spaces/parens
    if (rp) {
        char *lp = strchr(buf, '(');
        if (lp && rp > lp) {
            size_t clen = (size_t)(rp - lp - 1);
            if (clen >= sizeof(comm)) clen = sizeof(comm) - 1;
            memcpy(comm, lp + 1, clen);
            comm[clen] = '\0';
        }
        // fields after ")": state(1) ppid(2) ... starttime is field 22 overall,
        // i.e. the 20th token after comm.
        char *p = rp + 2;
        int field = 3;
        char *tok = strtok(p, " ");
        while (tok) {
            if (field == 4) ppid = atol(tok);
            if (field == 22) { starttime = strtoull(tok, NULL, 10); break; }
            tok = strtok(NULL, " ");
            field++;
        }
    }

    // Dedup key: pid + starttime (survives PID reuse).
    char key[64];
    snprintf(key, sizeof(key), "%s:%llu", pid, starttime);
    if (OSHash_Get(seen_procs, key)) {
        return;                              // already emitted
    }
    if (OSHash_Add(seen_procs, key, (void *)1) != 2) {
        // hash full or error: reset periodically to bound memory
        if (OSHash_Get(seen_procs, key) == NULL) {
            OSHash_Free(seen_procs);
            seen_procs = OSHash_Create();
            OSHash_Add(seen_procs, key, (void *)1);
        }
    }

    // /proc/<pid>/cmdline (null-separated args)
    snprintf(path, sizeof(path), "/proc/%s/cmdline", pid);
    f = fopen(path, "r");
    if (f) {
        size_t cn = fread(cmdline, 1, sizeof(cmdline) - 1, f);
        fclose(f);
        for (size_t i = 0; i < cn; i++) {
            if (cmdline[i] == '\0') cmdline[i] = ' ';
        }
        cmdline[cn] = '\0';
        // trim trailing space
        for (size_t i = cn; i > 0 && cmdline[i - 1] == ' '; i--) cmdline[i - 1] = '\0';
    }

    proc_user(pid, user, sizeof(user));

    char ts[32];
    time_t now = time(NULL);
    struct tm tmv;
    gmtime_r(&now, &tmv);
    strftime(ts, sizeof(ts), "%Y-%m-%dT%H:%M:%SZ", &tmv);

    cJSON *event = cJSON_CreateObject();
    cJSON_AddStringToObject(event, "collector", "edr_process");
    cJSON *data = cJSON_AddObjectToObject(event, "data");
    cJSON *proc = cJSON_AddObjectToObject(data, "process");
    cJSON_AddNumberToObject(proc, "pid", atoi(pid));
    cJSON *parent = cJSON_AddObjectToObject(proc, "parent");
    cJSON_AddNumberToObject(parent, "pid", ppid);
    cJSON_AddStringToObject(proc, "name", comm);
    cJSON_AddStringToObject(proc, "command_line", cmdline[0] ? cmdline : comm);
    cJSON_AddStringToObject(proc, "user", user);
    cJSON_AddStringToObject(proc, "start", ts);

    // Executable path + working directory (readlink) and SHA256 of the image.
    char exe[PATH_MAX] = "", cwd[PATH_MAX] = "";
    snprintf(path, sizeof(path), "/proc/%s/exe", pid);
    ssize_t el = readlink(path, exe, sizeof(exe) - 1);
    if (el > 0) {
        exe[el] = '\0';
        cJSON_AddStringToObject(proc, "executable", exe);
    }
    snprintf(path, sizeof(path), "/proc/%s/cwd", pid);
    ssize_t cl = readlink(path, cwd, sizeof(cwd) - 1);
    if (cl > 0) {
        cwd[cl] = '\0';
        cJSON_AddStringToObject(proc, "working_directory", cwd);
    }
    if (exe[0]) {
        os_sha256 sha = {0};
        if (OS_SHA256_File(exe, sha, OS_BINARY) == 0) {
            cJSON *h = cJSON_AddObjectToObject(proc, "hash");
            cJSON_AddStringToObject(h, "sha256", sha);
        }
    }
    // Script-execution + suspicious-command classification (MITRE T1059/T1105).
    if (basename_is_interpreter(comm)) {
        cJSON_AddBoolToObject(proc, "is_script", 1);
    }
    if (cmd_is_suspicious(cmdline)) {
        cJSON_AddBoolToObject(proc, "suspicious", 1);
    }

    wm_edr_emit(edr, event);
    cJSON_Delete(event);
}

static void scan_processes(wm_edr_t *edr) {
    DIR *d = opendir("/proc");
    if (!d) {
        return;
    }
    struct dirent *ent;
    while ((ent = readdir(d)) != NULL) {
        // numeric-only names are PIDs
        int numeric = 1;
        for (const char *c = ent->d_name; *c; c++) {
            if (!isdigit((unsigned char)*c)) { numeric = 0; break; }
        }
        if (numeric && ent->d_name[0]) {
            emit_process(edr, ent->d_name);
        }
    }
    closedir(d);
}

static void scan_network_file(wm_edr_t *edr, const char *file, int v6) {
    FILE *f = fopen(file, "r");
    if (!f) {
        return;
    }
    char line[512];
    int first = 1;
    while (fgets(line, sizeof(line), f)) {
        if (first) { first = 0; continue; }   // header
        char l_addr[128], r_addr[128];
        unsigned int st = 0;
        if (sscanf(line, "%*d: %127[0-9A-Fa-f:] %127[0-9A-Fa-f:] %x", l_addr, r_addr, &st) < 3) {
            continue;
        }
        if (st != 0x01 && st != 0x0A) {        // established or listening only
            continue;
        }
        char lip[64] = "", rip[64] = "";
        unsigned int lport = 0, rport = 0;
        char *lc = strrchr(l_addr, ':'), *rc = strrchr(r_addr, ':');
        if (!lc || !rc) continue;
        *lc = '\0'; *rc = '\0';
        lport = (unsigned int)strtoul(lc + 1, NULL, 16);
        rport = (unsigned int)strtoul(rc + 1, NULL, 16);
        if (v6) {
            snprintf(lip, sizeof(lip), "%s", "::");   // IPv6 rendering kept simple
            snprintf(rip, sizeof(rip), "%s", "::");
        } else {
            hex_to_ipv4(l_addr, lip, sizeof(lip));
            hex_to_ipv4(r_addr, rip, sizeof(rip));
        }

        char key[160];
        snprintf(key, sizeof(key), "%s:%u-%s:%u-%u", lip, lport, rip, rport, st);
        if (OSHash_Get(seen_conns, key)) {
            continue;
        }
        OSHash_Add(seen_conns, key, (void *)1);

        cJSON *event = cJSON_CreateObject();
        cJSON_AddStringToObject(event, "collector", "edr_network");
        cJSON *data = cJSON_AddObjectToObject(event, "data");
        cJSON *port = cJSON_AddObjectToObject(data, "port");
        cJSON_AddStringToObject(port, "protocol", v6 ? "tcp6" : "tcp");
        cJSON *local = cJSON_AddObjectToObject(port, "local");
        cJSON_AddStringToObject(local, "ip", lip);
        cJSON_AddNumberToObject(local, "port", lport);
        cJSON *remote = cJSON_AddObjectToObject(port, "remote");
        cJSON_AddStringToObject(remote, "ip", rip);
        cJSON_AddNumberToObject(remote, "port", rport);
        cJSON_AddStringToObject(port, "state", tcp_state(st));

        wm_edr_emit(edr, event);
        cJSON_Delete(event);
    }
    fclose(f);
}

static void scan_network(wm_edr_t *edr) {
    scan_network_file(edr, "/proc/net/tcp", 0);
    scan_network_file(edr, "/proc/net/tcp6", 1);
    // Bound memory: reset the connection cache if it grows large.
    if (seen_conns && OSHash_Get_Elem_ex(seen_conns) > 50000) {
        OSHash_Free(seen_conns);
        seen_conns = OSHash_Create();
    }
}

/* ---------------------------------------------------------- persistence sweep */

static void emit_persistence(wm_edr_t *edr, const char *type, const char *path_val) {
    if (!path_val || !path_val[0] || OSHash_Get(seen_persist, path_val)) {
        return;
    }
    if (OSHash_Add(seen_persist, path_val, (void *)1) != 2) {
        if (seen_persist && OSHash_Get_Elem_ex(seen_persist) > 20000) {
            OSHash_Free(seen_persist);
            seen_persist = OSHash_Create();
        }
        return;
    }
    char ts[32];
    time_t now = time(NULL);
    struct tm tmv;
    gmtime_r(&now, &tmv);
    strftime(ts, sizeof(ts), "%Y-%m-%dT%H:%M:%SZ", &tmv);

    cJSON *event = cJSON_CreateObject();
    cJSON_AddStringToObject(event, "collector", "edr_persistence");
    cJSON *data = cJSON_AddObjectToObject(event, "data");
    cJSON *p = cJSON_AddObjectToObject(data, "persistence");
    cJSON_AddStringToObject(p, "type", type);
    cJSON_AddStringToObject(p, "path", path_val);
    cJSON_AddStringToObject(p, "detected", ts);
    wm_edr_emit(edr, event);
    cJSON_Delete(event);
}

static void scan_persistence_dir(wm_edr_t *edr, const char *dir, const char *type) {
    DIR *d = opendir(dir);
    if (!d) {
        return;
    }
    struct dirent *ent;
    char full[PATH_MAX];
    while ((ent = readdir(d)) != NULL) {
        if (ent->d_name[0] == '.') continue;
        snprintf(full, sizeof(full), "%s/%s", dir, ent->d_name);
        emit_persistence(edr, type, full);
    }
    closedir(d);
}

// Sweep the common Linux persistence locations and emit an edr_persistence event
// for each newly-seen entry (cron, systemd units, startup scripts, shell rc).
static void scan_persistence(wm_edr_t *edr) {
    static const char *CRON[] = {"/etc/cron.d", "/etc/cron.daily", "/etc/cron.hourly",
                                 "/etc/cron.weekly", "/etc/cron.monthly",
                                 "/var/spool/cron/crontabs", "/var/spool/cron", NULL};
    for (int i = 0; CRON[i]; i++) scan_persistence_dir(edr, CRON[i], "cron");
    scan_persistence_dir(edr, "/etc/systemd/system", "systemd-service");
    scan_persistence_dir(edr, "/etc/profile.d", "shell-profile");

    static const char *RC[] = {"/root/.bashrc", "/root/.bash_profile", "/root/.profile",
                               "/etc/rc.local", "/etc/bash.bashrc", NULL};
    for (int i = 0; RC[i]; i++) {
        if (access(RC[i], F_OK) == 0) emit_persistence(edr, "shell-profile", RC[i]);
    }
}

/* --------------------------------------------------------------------- main */

void* wm_edr_main(wm_edr_t *edr) {
    if (!edr->flags.enabled) {
        mtinfo(WM_EDR_LOGTAG, "Module disabled. Exiting.");
        pthread_exit(NULL);
    }

    seen_procs = OSHash_Create();
    seen_conns = OSHash_Create();
    seen_persist = OSHash_Create();

    char log_path[PATH_MAX];
    snprintf(log_path, sizeof(log_path), "%s", WM_EDR_LOG_PATH);
    edr_log = fopen(log_path, "a");

    if ((queue_fd = StartMQ(DEFAULTQUEUE, WRITE, INFINITE_OPENQ_ATTEMPTS)) < 0) {
        mterror(WM_EDR_LOGTAG, "Can't connect to queue '%s'.", DEFAULTQUEUE);
    }

    edr->flags.running = 1;
    mtinfo(WM_EDR_LOGTAG, "Native EDR telemetry started (interval %us, processes=%u network=%u persistence=%u).",
           edr->interval, edr->flags.processes, edr->flags.network, edr->flags.persistence);

    unsigned int cycle = 0;
    while (edr->flags.running) {
        if (edr->flags.processes) {
            scan_processes(edr);
        }
        if (edr->flags.network) {
            scan_network(edr);
        }
        // Persistence changes slowly — sweep every ~12 cycles (bounded overhead).
        if (edr->flags.persistence && (cycle == 0 || cycle % 12 == 0)) {
            scan_persistence(edr);
        }
        cycle++;
        sleep(edr->interval);
    }

    return NULL;
}

static void wm_edr_destroy(wm_edr_t *edr) {
    if (edr_log) {
        fclose(edr_log);
        edr_log = NULL;
    }
    if (seen_procs) { OSHash_Free(seen_procs); seen_procs = NULL; }
    if (seen_conns) { OSHash_Free(seen_conns); seen_conns = NULL; }
    if (seen_persist) { OSHash_Free(seen_persist); seen_persist = NULL; }
    free(edr);
}

static cJSON *wm_edr_dump(const wm_edr_t *edr) {
    cJSON *root = cJSON_CreateObject();
    cJSON *wm_edr = cJSON_CreateObject();
    cJSON_AddStringToObject(wm_edr, "enabled", edr->flags.enabled ? "yes" : "no");
    cJSON_AddStringToObject(wm_edr, "processes", edr->flags.processes ? "yes" : "no");
    cJSON_AddStringToObject(wm_edr, "network", edr->flags.network ? "yes" : "no");
    cJSON_AddStringToObject(wm_edr, "persistence", edr->flags.persistence ? "yes" : "no");
    cJSON_AddNumberToObject(wm_edr, "interval", edr->interval);
    cJSON_AddNumberToObject(wm_edr, "max_eps", edr->max_eps);
    cJSON_AddItemToObject(root, "edr", wm_edr);
    return root;
}

#endif // !WIN32

#ifdef WIN32   /* ---------- Windows agent module (native Win32 telemetry) ---------- */

// This TU is compiled into guardsarm_modulesd_lib, which does not inherit the win32
// agent target's -D_WIN32_WINNT=0x600. QueryFullProcessImageNameA and inet_ntop are
// __stdcall (WINAPI) and only declared when _WIN32_WINNT >= 0x0600; without the
// prototype the compiler emits an implicit cdecl call and corrupts the stack. Force it.
#if !defined(_WIN32_WINNT) || (_WIN32_WINNT < 0x0600)
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <iphlpapi.h>

#include "wmodules.h"
#include "wm_edr.h"
#include "defs.h"
#include "mq_op.h"
#include "cJSON.h"
#include "sha256_op.h"

static void* wm_edr_main(wm_edr_t *edr);
static void wm_edr_destroy(wm_edr_t *edr);
static cJSON *wm_edr_dump(const wm_edr_t *edr);

const wm_context WM_EDR_CONTEXT = {
    .name = WM_EDR_CONTEXT_NAME,
    .start = (wm_routine)wm_edr_main,
    .destroy = (void(*)(void *))wm_edr_destroy,
    .dump = (cJSON *(*)(const void *))wm_edr_dump,
    .sync = NULL,
    .stop = NULL,
    .query = NULL,
};

static int queue_fd = 0;
static FILE *edr_log = NULL;
static OSHash *seen_procs = NULL;
static OSHash *seen_conns = NULL;
static OSHash *seen_persist = NULL;

static void now_iso(char *ts, size_t n) {
    time_t now = time(NULL);
    struct tm *t = gmtime(&now);
    if (t) {
        strftime(ts, n, "%Y-%m-%dT%H:%M:%SZ", t);
    } else {
        ts[0] = '\0';
    }
}

static void wm_edr_emit(wm_edr_t *edr, cJSON *event) {
    char *msg = cJSON_PrintUnformatted(event);
    if (!msg) {
        return;
    }
    const int eps = edr->max_eps > 0 ? (1000000 / (int)edr->max_eps) : 0;
    if (wm_sendmsg(eps, queue_fd, msg, WM_EDR_CONTEXT_NAME, LOCALFILE_MQ) < 0) {
        mterror(WM_EDR_LOGTAG, "Unable to send message to '%s'", DEFAULTQUEUE);
        if ((queue_fd = StartMQ(DEFAULTQUEUE, WRITE, 1)) >= 0) {
            wm_sendmsg(eps, queue_fd, msg, WM_EDR_CONTEXT_NAME, LOCALFILE_MQ);
        }
    }
    if (edr_log) {
        fprintf(edr_log, "%s\n", msg);
        fflush(edr_log);
    }
    free(msg);
}

// A process whose executable basename is a known interpreter / LOLBin (MITRE T1059).
static int is_interpreter_win(const char *name) {
    static const char *I[] = {"powershell.exe", "pwsh.exe", "cmd.exe", "wscript.exe",
                              "cscript.exe", "mshta.exe", "python.exe", "python3.exe",
                              "wmic.exe", "rundll32.exe", "regsvr32.exe", "bitsadmin.exe",
                              "certutil.exe", "msbuild.exe", "installutil.exe", NULL};
    for (int i = 0; I[i]; i++) {
        if (!_stricmp(name, I[i])) {
            return 1;
        }
    }
    return 0;
}

// Resolve the token user of a process handle to "DOMAIN\\user".
static void user_of_process(HANDLE hProc, char *out, size_t n) {
    out[0] = '\0';
    HANDLE tok = NULL;
    if (!OpenProcessToken(hProc, TOKEN_QUERY, &tok)) {
        return;
    }
    DWORD len = 0;
    GetTokenInformation(tok, TokenUser, NULL, 0, &len);
    if (len) {
        TOKEN_USER *tu = (TOKEN_USER *)malloc(len);
        if (tu && GetTokenInformation(tok, TokenUser, tu, len, &len)) {
            char name[256] = "", dom[256] = "";
            DWORD nl = sizeof(name), dl = sizeof(dom);
            SID_NAME_USE use;
            if (LookupAccountSidA(NULL, tu->User.Sid, name, &nl, dom, &dl, &use)) {
                snprintf(out, n, "%s\\%s", dom, name);
            }
        }
        free(tu);
    }
    CloseHandle(tok);
}

// Emit an edr_process event for a newly-seen (pid,createtime) process.
static void emit_process_win(wm_edr_t *edr, DWORD pid, DWORD ppid, const char *name) {
    if (pid == 0) {
        return;
    }
    HANDLE hProc = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    unsigned long long start_ull = 0;
    char exe[MAX_PATH] = "", user[256] = "", startiso[32] = "";

    if (hProc) {
        DWORD sz = MAX_PATH;
        QueryFullProcessImageNameA(hProc, 0, exe, &sz);
        FILETIME ct, et, kt, ut;
        if (GetProcessTimes(hProc, &ct, &et, &kt, &ut)) {
            ULARGE_INTEGER u;
            u.LowPart = ct.dwLowDateTime;
            u.HighPart = ct.dwHighDateTime;
            start_ull = u.QuadPart;
            SYSTEMTIME s;
            if (FileTimeToSystemTime(&ct, &s)) {
                snprintf(startiso, sizeof(startiso), "%04d-%02d-%02dT%02d:%02d:%02dZ",
                         s.wYear, s.wMonth, s.wDay, s.wHour, s.wMinute, s.wSecond);
            }
        }
        user_of_process(hProc, user, sizeof(user));
    }

    // Dedup by pid + creation time (survives PID reuse).
    char key[64];
    snprintf(key, sizeof(key), "%lu:%llu", (unsigned long)pid, start_ull);
    if (OSHash_Get(seen_procs, key)) {
        if (hProc) CloseHandle(hProc);
        return;
    }
    if (OSHash_Add(seen_procs, key, (void *)1) != 2) {
        if (OSHash_Get(seen_procs, key) == NULL) {
            OSHash_Free(seen_procs);
            seen_procs = OSHash_Create();
            OSHash_Add(seen_procs, key, (void *)1);
        }
    }
    if (!startiso[0]) {
        now_iso(startiso, sizeof(startiso));
    }

    cJSON *event = cJSON_CreateObject();
    cJSON_AddStringToObject(event, "collector", "edr_process");
    cJSON *data = cJSON_AddObjectToObject(event, "data");
    cJSON *proc = cJSON_AddObjectToObject(data, "process");
    cJSON_AddNumberToObject(proc, "pid", (double)pid);
    cJSON *parent = cJSON_AddObjectToObject(proc, "parent");
    cJSON_AddNumberToObject(parent, "pid", (double)ppid);
    cJSON_AddStringToObject(proc, "name", name);
    cJSON_AddStringToObject(proc, "command_line", exe[0] ? exe : name);
    cJSON_AddStringToObject(proc, "user", user);
    cJSON_AddStringToObject(proc, "start", startiso);
    if (exe[0]) {
        cJSON_AddStringToObject(proc, "executable", exe);
        os_sha256 sha = {0};
        if (OS_SHA256_File(exe, sha, OS_BINARY) == 0) {
            cJSON *h = cJSON_AddObjectToObject(proc, "hash");
            cJSON_AddStringToObject(h, "sha256", sha);
        }
    }
    if (is_interpreter_win(name)) {
        cJSON_AddBoolToObject(proc, "is_script", 1);
    }

    wm_edr_emit(edr, event);
    cJSON_Delete(event);
    if (hProc) CloseHandle(hProc);
}

static void scan_processes(wm_edr_t *edr) {
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) {
        return;
    }
    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(pe);
    if (Process32First(snap, &pe)) {
        do {
            emit_process_win(edr, pe.th32ProcessID, pe.th32ParentProcessID, pe.szExeFile);
        } while (Process32Next(snap, &pe));
    }
    CloseHandle(snap);
}

static const char *tcp_state_win(DWORD s) {
    switch (s) {
        case MIB_TCP_STATE_ESTAB: return "established";
        case MIB_TCP_STATE_LISTEN: return "listening";
        case MIB_TCP_STATE_SYN_SENT: return "syn_sent";
        case MIB_TCP_STATE_SYN_RCVD: return "syn_recv";
        case MIB_TCP_STATE_TIME_WAIT: return "time_wait";
        case MIB_TCP_STATE_CLOSE_WAIT: return "close_wait";
        default: return "other";
    }
}

static void emit_conn(wm_edr_t *edr, const char *proto, const char *lip, unsigned lport,
                      const char *rip, unsigned rport, const char *state, DWORD pid) {
    char key[160];
    snprintf(key, sizeof(key), "%s-%s:%u-%s:%u", proto, lip, lport, rip, rport);
    if (OSHash_Get(seen_conns, key)) {
        return;
    }
    OSHash_Add(seen_conns, key, (void *)1);

    cJSON *event = cJSON_CreateObject();
    cJSON_AddStringToObject(event, "collector", "edr_network");
    cJSON *data = cJSON_AddObjectToObject(event, "data");
    cJSON *port = cJSON_AddObjectToObject(data, "port");
    cJSON_AddStringToObject(port, "protocol", proto);
    cJSON *local = cJSON_AddObjectToObject(port, "local");
    cJSON_AddStringToObject(local, "ip", lip);
    cJSON_AddNumberToObject(local, "port", lport);
    cJSON *remote = cJSON_AddObjectToObject(port, "remote");
    cJSON_AddStringToObject(remote, "ip", rip);
    cJSON_AddNumberToObject(remote, "port", rport);
    cJSON_AddStringToObject(port, "state", state);
    cJSON *proc = cJSON_AddObjectToObject(data, "process");
    cJSON_AddNumberToObject(proc, "pid", (double)pid);
    wm_edr_emit(edr, event);
    cJSON_Delete(event);
}

static void scan_network(wm_edr_t *edr) {
    // IPv4 TCP with owning PID.
    DWORD sz = 0;
    GetExtendedTcpTable(NULL, &sz, FALSE, AF_INET, TCP_TABLE_OWNER_PID_ALL, 0);
    if (sz) {
        MIB_TCPTABLE_OWNER_PID *t = (MIB_TCPTABLE_OWNER_PID *)malloc(sz);
        if (t && GetExtendedTcpTable(t, &sz, FALSE, AF_INET, TCP_TABLE_OWNER_PID_ALL, 0) == NO_ERROR) {
            for (DWORD i = 0; i < t->dwNumEntries; i++) {
                MIB_TCPROW_OWNER_PID *r = &t->table[i];
                if (r->dwState != MIB_TCP_STATE_ESTAB && r->dwState != MIB_TCP_STATE_LISTEN) {
                    continue;
                }
                unsigned char *la = (unsigned char *)&r->dwLocalAddr;
                unsigned char *ra = (unsigned char *)&r->dwRemoteAddr;
                char lip[64], rip[64];
                snprintf(lip, sizeof(lip), "%u.%u.%u.%u", la[0], la[1], la[2], la[3]);
                snprintf(rip, sizeof(rip), "%u.%u.%u.%u", ra[0], ra[1], ra[2], ra[3]);
                unsigned lport = ntohs((u_short)r->dwLocalPort);
                unsigned rport = ntohs((u_short)r->dwRemotePort);
                emit_conn(edr, "tcp", lip, lport, rip, rport, tcp_state_win(r->dwState), r->dwOwningPid);
            }
        }
        free(t);
    }
    // IPv6 TCP with owning PID.
    sz = 0;
    GetExtendedTcpTable(NULL, &sz, FALSE, AF_INET6, TCP_TABLE_OWNER_PID_ALL, 0);
    if (sz) {
        MIB_TCP6TABLE_OWNER_PID *t = (MIB_TCP6TABLE_OWNER_PID *)malloc(sz);
        if (t && GetExtendedTcpTable(t, &sz, FALSE, AF_INET6, TCP_TABLE_OWNER_PID_ALL, 0) == NO_ERROR) {
            for (DWORD i = 0; i < t->dwNumEntries; i++) {
                MIB_TCP6ROW_OWNER_PID *r = &t->table[i];
                if (r->dwState != MIB_TCP_STATE_ESTAB && r->dwState != MIB_TCP_STATE_LISTEN) {
                    continue;
                }
                char lip[64] = "", rip[64] = "";
                inet_ntop(AF_INET6, r->ucLocalAddr, lip, sizeof(lip));
                inet_ntop(AF_INET6, r->ucRemoteAddr, rip, sizeof(rip));
                unsigned lport = ntohs((u_short)r->dwLocalPort);
                unsigned rport = ntohs((u_short)r->dwRemotePort);
                emit_conn(edr, "tcp6", lip, lport, rip, rport, tcp_state_win(r->dwState), r->dwOwningPid);
            }
        }
        free(t);
    }
    if (seen_conns && OSHash_Get_Elem_ex(seen_conns) > 50000) {
        OSHash_Free(seen_conns);
        seen_conns = OSHash_Create();
    }
}

/* ---------------------------------------------------------- persistence sweep */

static void emit_persistence(wm_edr_t *edr, const char *type, const char *path_val) {
    if (!path_val || !path_val[0] || OSHash_Get(seen_persist, path_val)) {
        return;
    }
    if (OSHash_Add(seen_persist, path_val, (void *)1) != 2) {
        if (seen_persist && OSHash_Get_Elem_ex(seen_persist) > 20000) {
            OSHash_Free(seen_persist);
            seen_persist = OSHash_Create();
        }
        return;
    }
    char ts[32];
    now_iso(ts, sizeof(ts));
    cJSON *event = cJSON_CreateObject();
    cJSON_AddStringToObject(event, "collector", "edr_persistence");
    cJSON *data = cJSON_AddObjectToObject(event, "data");
    cJSON *p = cJSON_AddObjectToObject(data, "persistence");
    cJSON_AddStringToObject(p, "type", type);
    cJSON_AddStringToObject(p, "path", path_val);
    cJSON_AddStringToObject(p, "detected", ts);
    wm_edr_emit(edr, event);
    cJSON_Delete(event);
}

// Enumerate the string values of an autostart registry key (Run / RunOnce).
static void scan_reg_run(wm_edr_t *edr, HKEY root, const char *subkey, const char *label) {
    HKEY hk;
    if (RegOpenKeyExA(root, subkey, 0, KEY_READ | KEY_WOW64_64KEY, &hk) != ERROR_SUCCESS) {
        return;
    }
    DWORD idx = 0;
    char name[256], val[1024];
    while (1) {
        DWORD nl = sizeof(name), vl = sizeof(val), type = 0;
        if (RegEnumValueA(hk, idx++, name, &nl, NULL, &type, (LPBYTE)val, &vl) != ERROR_SUCCESS) {
            break;
        }
        if (type == REG_SZ || type == REG_EXPAND_SZ) {
            char path[1400];
            snprintf(path, sizeof(path), "%s\\%s => %s", label, name, val);
            emit_persistence(edr, "registry-run", path);
        }
    }
    RegCloseKey(hk);
}

// Enumerate files in a Startup folder.
static void scan_startup_dir(wm_edr_t *edr, const char *dir) {
    char pattern[MAX_PATH];
    snprintf(pattern, sizeof(pattern), "%s\\*", dir);
    WIN32_FIND_DATAA fd;
    HANDLE h = FindFirstFileA(pattern, &fd);
    if (h == INVALID_HANDLE_VALUE) {
        return;
    }
    do {
        if (fd.cFileName[0] == '.') {
            continue;
        }
        char full[MAX_PATH];
        snprintf(full, sizeof(full), "%s\\%s", dir, fd.cFileName);
        emit_persistence(edr, "startup-folder", full);
    } while (FindNextFileA(h, &fd));
    FindClose(h);
}

static void scan_persistence(wm_edr_t *edr) {
    scan_reg_run(edr, HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Windows\\CurrentVersion\\Run", "HKLM\\...\\Run");
    scan_reg_run(edr, HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Windows\\CurrentVersion\\RunOnce", "HKLM\\...\\RunOnce");
    scan_reg_run(edr, HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Run", "HKCU\\...\\Run");
    scan_reg_run(edr, HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\RunOnce", "HKCU\\...\\RunOnce");

    char *pd = getenv("ProgramData");
    if (pd) {
        char sd[MAX_PATH];
        snprintf(sd, sizeof(sd), "%s\\Microsoft\\Windows\\Start Menu\\Programs\\StartUp", pd);
        scan_startup_dir(edr, sd);
    }
    char *ap = getenv("APPDATA");
    if (ap) {
        char sd[MAX_PATH];
        snprintf(sd, sizeof(sd), "%s\\Microsoft\\Windows\\Start Menu\\Programs\\Startup", ap);
        scan_startup_dir(edr, sd);
    }
}

/* --------------------------------------------------------------------- main */

void* wm_edr_main(wm_edr_t *edr) {
    if (!edr->flags.enabled) {
        mtinfo(WM_EDR_LOGTAG, "Module disabled. Exiting.");
        return NULL;
    }

    seen_procs = OSHash_Create();
    seen_conns = OSHash_Create();
    seen_persist = OSHash_Create();

    edr_log = fopen(WM_EDR_LOG_PATH, "a");

    if ((queue_fd = StartMQ(DEFAULTQUEUE, WRITE, INFINITE_OPENQ_ATTEMPTS)) < 0) {
        mterror(WM_EDR_LOGTAG, "Can't connect to queue '%s'.", DEFAULTQUEUE);
    }

    edr->flags.running = 1;
    mtinfo(WM_EDR_LOGTAG, "Native EDR telemetry started (interval %us, processes=%u network=%u persistence=%u).",
           edr->interval, edr->flags.processes, edr->flags.network, edr->flags.persistence);

    unsigned int cycle = 0;
    while (edr->flags.running) {
        if (edr->flags.processes) {
            scan_processes(edr);
        }
        if (edr->flags.network) {
            scan_network(edr);
        }
        if (edr->flags.persistence && (cycle == 0 || cycle % 12 == 0)) {
            scan_persistence(edr);
        }
        cycle++;
        Sleep(edr->interval * 1000);
    }

    return NULL;
}

static void wm_edr_destroy(wm_edr_t *edr) {
    if (edr_log) {
        fclose(edr_log);
        edr_log = NULL;
    }
    if (seen_procs) { OSHash_Free(seen_procs); seen_procs = NULL; }
    if (seen_conns) { OSHash_Free(seen_conns); seen_conns = NULL; }
    if (seen_persist) { OSHash_Free(seen_persist); seen_persist = NULL; }
    free(edr);
}

static cJSON *wm_edr_dump(const wm_edr_t *edr) {
    cJSON *root = cJSON_CreateObject();
    cJSON *wm_edr = cJSON_CreateObject();
    cJSON_AddStringToObject(wm_edr, "enabled", edr->flags.enabled ? "yes" : "no");
    cJSON_AddStringToObject(wm_edr, "processes", edr->flags.processes ? "yes" : "no");
    cJSON_AddStringToObject(wm_edr, "network", edr->flags.network ? "yes" : "no");
    cJSON_AddStringToObject(wm_edr, "persistence", edr->flags.persistence ? "yes" : "no");
    cJSON_AddNumberToObject(wm_edr, "interval", edr->interval);
    cJSON_AddNumberToObject(wm_edr, "max_eps", edr->max_eps);
    cJSON_AddItemToObject(root, "edr", wm_edr);
    return root;
}

#endif // WIN32
