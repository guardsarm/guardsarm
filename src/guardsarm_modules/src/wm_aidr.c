/*
 * GuardSarm Module for native AIDR (AI Detection & Response) telemetry
 * Copyright (C) 2026 GuardSarm, Inc.
 *
 * See wm_aidr.h for a description. Native, self-contained: no external
 * library, no dbsync — a lightweight host sweeper that streams JSON AI-surface
 * telemetry (local LLM runtimes, LLM API egress, MCP servers, model
 * artifacts) to the manager through the standard agent queue (and mirrors it
 * to a local log the host applier can ingest). Mirrors the native EDR module
 * (wm_edr.c) structure: Linux (/proc) implementation first, Windows (Win32)
 * implementation second.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized
 * copying, distribution, modification, or use is prohibited except under a
 * written license agreement with GuardSarm, Inc.
 */

#ifndef WIN32   // Linux/Unix agent module (native /proc telemetry).

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <unistd.h>
#include <time.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "wmodules.h"
#include "wm_aidr.h"
#include "wm_aidr_classify.h"
#include "defs.h"
#include "mq_op.h"
#include "cJSON.h"
#include "sha256_op.h"

static void* gm_aidr_main(gm_aidr_t *aidr);          // Module main function. It won't return
static void gm_aidr_destroy(gm_aidr_t *aidr);        // Destroy configuration
static cJSON *gm_aidr_dump(const gm_aidr_t *aidr);   // Dump current configuration

const gm_context GM_AIDR_CONTEXT = {
    .name = GM_AIDR_CONTEXT_NAME,
    .start = (gm_routine)gm_aidr_main,
    .destroy = (void(*)(void *))gm_aidr_destroy,
    .dump = (cJSON *(*)(const void *))gm_aidr_dump,
    .sync = NULL,
    .stop = NULL,
    .query = NULL,
};

static int queue_fd = 0;                             // Output queue file descriptor
static FILE *aidr_log = NULL;                        // Mirror log for the host applier
static OSHash *seen_runtime = NULL;                  // "pid:name" -> emitted
static OSHash *seen_api = NULL;                      // "pid:rip:rport" -> emitted
static OSHash *seen_mcp = NULL;                      // process pid / config path:server -> emitted
static OSHash *seen_model = NULL;                    // model path -> emitted

/* ------------------------------------------------------------------ sockets */

#define AIDR_MAX_LISTEN  256
#define AIDR_MAX_ESTAB   1024
#define AIDR_MAX_PROV_IP 256
#define AIDR_MAX_MODELS  32

typedef struct { unsigned long inode; unsigned int port; } aidr_lsock_t;
typedef struct { unsigned long inode; char rip[46]; unsigned int rport; } aidr_esock_t;

static aidr_lsock_t lsocks[AIDR_MAX_LISTEN];
static int n_lsocks = 0;
static aidr_esock_t esocks[AIDR_MAX_ESTAB];
static int n_esocks = 0;

// Resolved provider endpoint table (host -> A/AAAA records, refreshed hourly).
typedef struct { char ip[46]; const char *host; const char *provider; } aidr_previp_t;
static aidr_previp_t prov_ips[AIDR_MAX_PROV_IP];
static int n_prov_ips = 0;
static time_t prov_resolved_at = 0;

// Convert a /proc/net little-endian hex IPv4 (e.g. "0100007F") to dotted quad.
static void aidr_hex_to_ipv4(const char *hex, char *out, size_t out_sz) {
    unsigned int a = 0, b = 0, c = 0, d = 0;
    if (sscanf(hex, "%2x%2x%2x%2x", &d, &c, &b, &a) == 4) {
        snprintf(out, out_sz, "%u.%u.%u.%u", a, b, c, d);
    } else {
        snprintf(out, out_sz, "0.0.0.0");
    }
}

// Convert a /proc/net/tcp6 hex address (4 little-endian 32-bit words) to text.
static void aidr_hex_to_ipv6(const char *hex, char *out, size_t out_sz) {
    struct in6_addr a6;
    unsigned char *b = a6.s6_addr;
    out[0] = '\0';
    if (strlen(hex) < 32) {
        snprintf(out, out_sz, "::");
        return;
    }
    for (int w = 0; w < 4; w++) {
        unsigned int word = 0;
        if (sscanf(hex + w * 8, "%8x", &word) != 1) {
            snprintf(out, out_sz, "::");
            return;
        }
        // each 32-bit word is stored little-endian
        b[w * 4 + 0] = word & 0xFF;
        b[w * 4 + 1] = (word >> 8) & 0xFF;
        b[w * 4 + 2] = (word >> 16) & 0xFF;
        b[w * 4 + 3] = (word >> 24) & 0xFF;
    }
    if (!inet_ntop(AF_INET6, &a6, out, out_sz)) {
        snprintf(out, out_sz, "::");
    }
}

// Read /proc/net/tcp[6] once into the listen + established socket tables.
static void aidr_scan_net_file(const char *file, int v6) {
    FILE *f = fopen(file, "r");
    if (!f) {
        return;
    }
    char line[768];
    int first = 1;
    while (fgets(line, sizeof(line), f)) {
        if (first) { first = 0; continue; }
        char l_addr[128], r_addr[128];
        unsigned int st = 0;
        unsigned long inode = 0;
        // sl local rem st tx_queue:rx_queue tr:tm->when retrnsmt uid timeout inode
        if (sscanf(line, "%*d: %127[0-9A-Fa-f:] %127[0-9A-Fa-f:] %x %*s %*s %*s %*d %*d %lu",
                   l_addr, r_addr, &st, &inode) < 4) {
            continue;
        }
        char *lc = strrchr(l_addr, ':'), *rc = strrchr(r_addr, ':');
        if (!lc || !rc) {
            continue;
        }
        *lc = '\0'; *rc = '\0';
        unsigned int lport = (unsigned int)strtoul(lc + 1, NULL, 16);
        unsigned int rport = (unsigned int)strtoul(rc + 1, NULL, 16);
        if (st == 0x0A && n_lsocks < AIDR_MAX_LISTEN) {          // LISTEN
            lsocks[n_lsocks].inode = inode;
            lsocks[n_lsocks].port = lport;
            n_lsocks++;
        } else if (st == 0x01 && n_esocks < AIDR_MAX_ESTAB) {    // ESTABLISHED
            esocks[n_esocks].inode = inode;
            esocks[n_esocks].rport = rport;
            if (v6) {
                aidr_hex_to_ipv6(r_addr, esocks[n_esocks].rip, sizeof(esocks[n_esocks].rip));
            } else {
                aidr_hex_to_ipv4(r_addr, esocks[n_esocks].rip, sizeof(esocks[n_esocks].rip));
            }
            n_esocks++;
        }
    }
    fclose(f);
}

static void aidr_refresh_sockets(void) {
    n_lsocks = 0;
    n_esocks = 0;
    aidr_scan_net_file("/proc/net/tcp", 0);
    aidr_scan_net_file("/proc/net/tcp6", 1);
}

// Resolve the known AI endpoint hosts to IPs (refreshed hourly). Real DNS —
// resolution failures simply leave fewer entries (no egress matching then).
static void aidr_refresh_providers(void) {
    time_t now = time(NULL);
    if (n_prov_ips > 0 && now - prov_resolved_at < 3600) {
        return;
    }
    n_prov_ips = 0;
    for (int i = 0; AIDR_PROVIDER_HOSTS[i].host && n_prov_ips < AIDR_MAX_PROV_IP; i++) {
        struct addrinfo hints, *res = NULL, *rp;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        if (getaddrinfo(AIDR_PROVIDER_HOSTS[i].host, "443", &hints, &res) != 0) {
            continue;
        }
        int per_host = 0;
        for (rp = res; rp && per_host < 4 && n_prov_ips < AIDR_MAX_PROV_IP; rp = rp->ai_next) {
            char ip[46] = "";
            if (rp->ai_family == AF_INET) {
                struct sockaddr_in *sa = (struct sockaddr_in *)rp->ai_addr;
                inet_ntop(AF_INET, &sa->sin_addr, ip, sizeof(ip));
            } else if (rp->ai_family == AF_INET6) {
                struct sockaddr_in6 *sa6 = (struct sockaddr_in6 *)rp->ai_addr;
                inet_ntop(AF_INET6, &sa6->sin6_addr, ip, sizeof(ip));
            }
            if (ip[0]) {
                snprintf(prov_ips[n_prov_ips].ip, sizeof(prov_ips[n_prov_ips].ip), "%s", ip);
                prov_ips[n_prov_ips].host = AIDR_PROVIDER_HOSTS[i].host;
                prov_ips[n_prov_ips].provider = AIDR_PROVIDER_HOSTS[i].provider;
                n_prov_ips++;
                per_host++;
            }
        }
        freeaddrinfo(res);
    }
    prov_resolved_at = now;
}

static const aidr_previp_t *aidr_prov_for_ip(const char *ip) {
    for (int i = 0; i < n_prov_ips; i++) {
        if (!strcmp(prov_ips[i].ip, ip)) {
            return &prov_ips[i];
        }
    }
    return NULL;
}

// First LISTEN port owned by `pid` (scan its /proc/<pid>/fd for socket inodes).
static unsigned int aidr_pid_listen_port(const char *pid) {
    if (n_lsocks == 0) {
        return 0;
    }
    char fddir[280];
    snprintf(fddir, sizeof(fddir), "/proc/%s/fd", pid);
    DIR *d = opendir(fddir);
    if (!d) {
        return 0;
    }
    struct dirent *ent;
    unsigned int port = 0;
    while (!port && (ent = readdir(d)) != NULL) {
        if (ent->d_name[0] == '.') continue;
        char lpath[560], target[128];
        snprintf(lpath, sizeof(lpath), "%s/%s", fddir, ent->d_name);
        ssize_t n = readlink(lpath, target, sizeof(target) - 1);
        if (n <= 0) continue;
        target[n] = '\0';
        unsigned long inode = 0;
        if (sscanf(target, "socket:[%lu]", &inode) != 1) continue;
        for (int i = 0; i < n_lsocks; i++) {
            if (lsocks[i].inode == inode) {
                port = lsocks[i].port;
                break;
            }
        }
    }
    closedir(d);
    return port;
}

/* -------------------------------------------------------------------- procs */

// Read /proc/<pid>/comm-style name from /proc/<pid>/stat plus the cmdline and
// exe path. Returns 0 on failure.
static int aidr_read_proc(const char *pid, char *comm, size_t comm_sz,
                          char *cmdline, size_t cmd_sz, char *exe, size_t exe_sz) {
    char path[280];
    comm[0] = cmdline[0] = exe[0] = '\0';

    snprintf(path, sizeof(path), "/proc/%s/stat", pid);
    FILE *f = fopen(path, "r");
    if (!f) {
        return 0;
    }
    char buf[4096];
    size_t n = fread(buf, 1, sizeof(buf) - 1, f);
    fclose(f);
    if (n == 0) {
        return 0;
    }
    buf[n] = '\0';
    char *lp = strchr(buf, '('), *rp = strrchr(buf, ')');
    if (lp && rp && rp > lp) {
        size_t clen = (size_t)(rp - lp - 1);
        if (clen >= comm_sz) clen = comm_sz - 1;
        memcpy(comm, lp + 1, clen);
        comm[clen] = '\0';
    }

    snprintf(path, sizeof(path), "/proc/%s/cmdline", pid);
    f = fopen(path, "r");
    if (f) {
        size_t cn = fread(cmdline, 1, cmd_sz - 1, f);
        fclose(f);
        for (size_t i = 0; i < cn; i++) {
            if (cmdline[i] == '\0') cmdline[i] = ' ';
        }
        cmdline[cn] = '\0';
        for (size_t i = cn; i > 0 && cmdline[i - 1] == ' '; i--) cmdline[i - 1] = '\0';
    }

    snprintf(path, sizeof(path), "/proc/%s/exe", pid);
    ssize_t el = readlink(path, exe, exe_sz - 1);
    if (el > 0) {
        exe[el] = '\0';
    } else {
        exe[0] = '\0';
    }
    return 1;
}

/* --------------------------------------------------------------------- emit */

// Send one JSON event to the manager queue AND mirror it to the applier log.
static void gm_aidr_emit(gm_aidr_t *aidr, cJSON *event) {
    char *msg = cJSON_PrintUnformatted(event);
    if (!msg) {
        return;
    }
    const int eps = aidr->max_eps > 0 ? (1000000 / (int)aidr->max_eps) : 0;
    if (gm_sendmsg(eps, queue_fd, msg, GM_AIDR_CONTEXT_NAME, LOCALFILE_MQ) < 0) {
        mterror(GM_AIDR_LOGTAG, "Unable to send message to '%s'", DEFAULTQUEUE);
        if ((queue_fd = StartMQ(DEFAULTQUEUE, WRITE, 1)) >= 0) {
            gm_sendmsg(eps, queue_fd, msg, GM_AIDR_CONTEXT_NAME, LOCALFILE_MQ);
        }
    }
    if (aidr_log) {
        fprintf(aidr_log, "%s\n", msg);
        fflush(aidr_log);
    }
    free(msg);
}

static cJSON *aidr_new_event(const char *event_type) {
    cJSON *event = cJSON_CreateObject();
    cJSON_AddStringToObject(event, "collector", "aidr");
    cJSON_AddStringToObject(event, "event_type", event_type);
    return event;
}

/* --------------------------------------------------------------- ai_runtime */

// Collect Ollama model names from the on-disk manifest tree
// <root>/models/manifests/<registry>/<ns>/<model>/<tag> -> "model:tag".
static void aidr_ollama_manifests(const char *root, cJSON *models, int *count) {
    char mdir[PATH_MAX];
    snprintf(mdir, sizeof(mdir), "%s/models/manifests", root);
    DIR *dreg = opendir(mdir);
    if (!dreg) {
        return;
    }
    struct dirent *reg;
    while ((reg = readdir(dreg)) != NULL && *count < AIDR_MAX_MODELS) {
        if (reg->d_name[0] == '.') continue;
        char nsdir[PATH_MAX];
        snprintf(nsdir, sizeof(nsdir), "%s/%s", mdir, reg->d_name);
        DIR *dns = opendir(nsdir);
        if (!dns) continue;
        struct dirent *ns;
        while ((ns = readdir(dns)) != NULL && *count < AIDR_MAX_MODELS) {
            if (ns->d_name[0] == '.') continue;
            char modeldir[PATH_MAX];
            snprintf(modeldir, sizeof(modeldir), "%s/%s", nsdir, ns->d_name);
            DIR *dm = opendir(modeldir);
            if (!dm) continue;
            struct dirent *m;
            while ((m = readdir(dm)) != NULL && *count < AIDR_MAX_MODELS) {
                if (m->d_name[0] == '.') continue;
                char tagdir[PATH_MAX];
                snprintf(tagdir, sizeof(tagdir), "%s/%s", modeldir, m->d_name);
                DIR *dt = opendir(tagdir);
                if (!dt) continue;
                struct dirent *t;
                while ((t = readdir(dt)) != NULL && *count < AIDR_MAX_MODELS) {
                    if (t->d_name[0] == '.') continue;
                    char name[512];
                    snprintf(name, sizeof(name), "%s:%s", m->d_name, t->d_name);
                    cJSON_AddItemToArray(models, cJSON_CreateString(name));
                    (*count)++;
                }
                closedir(dt);
            }
            closedir(dm);
        }
        closedir(dns);
    }
    closedir(dreg);
}

// Pull "-m <path>" / "--model <path>" from a command line into the model list.
static void aidr_models_from_cmdline(const char *cmdline, cJSON *models, int *count) {
    if (!cmdline || !cmdline[0] || *count >= AIDR_MAX_MODELS) {
        return;
    }
    const char *flags[] = {" -m ", " --model ", " --model-path ", NULL};
    for (int i = 0; flags[i]; i++) {
        const char *p = aidr_stristr(cmdline, flags[i]);
        if (!p) continue;
        p += strlen(flags[i]);
        while (*p == ' ') p++;
        char val[512];
        size_t n = 0;
        while (p[n] && p[n] != ' ' && n < sizeof(val) - 1) {
            val[n] = p[n];
            n++;
        }
        val[n] = '\0';
        if (n) {
            const char *base = strrchr(val, '/');
            cJSON_AddItemToArray(models, cJSON_CreateString(base ? base + 1 : val));
            (*count)++;
        }
    }
}

// Home directories to sweep for AI artifacts / configs: /root + /home/*.
#define AIDR_MAX_HOMES 32
static int aidr_homes(char homes[][PATH_MAX]) {
    int n = 0;
    snprintf(homes[n++], PATH_MAX, "/root");
    DIR *d = opendir("/home");
    if (d) {
        struct dirent *ent;
        while ((ent = readdir(d)) != NULL && n < AIDR_MAX_HOMES) {
            if (ent->d_name[0] == '.') continue;
            snprintf(homes[n++], PATH_MAX, "/home/%s", ent->d_name);
        }
        closedir(d);
    }
    return n;
}

static void aidr_emit_runtime(gm_aidr_t *aidr, const char *pid, const char *name,
                              const char *exe, const char *cmdline) {
    char key[320];
    snprintf(key, sizeof(key), "rt:%s:%s", pid, name);
    if (OSHash_Get(seen_runtime, key)) {
        return;
    }
    OSHash_Add(seen_runtime, key, (void *)1);

    cJSON *event = aidr_new_event("ai_runtime");
    cJSON *data = cJSON_AddObjectToObject(event, "data");
    cJSON *rt = cJSON_AddObjectToObject(data, "runtime");
    cJSON_AddStringToObject(rt, "name", name);
    cJSON_AddNumberToObject(rt, "pid", atoi(pid));
    if (exe && exe[0]) {
        cJSON_AddStringToObject(rt, "exe_path", exe);
    }
    unsigned int port = aidr_pid_listen_port(pid);
    if (port) {
        cJSON_AddNumberToObject(rt, "listen_port", port);
    }
    cJSON *models = cJSON_AddArrayToObject(rt, "models_loaded");
    int mcount = 0;
    aidr_models_from_cmdline(cmdline, models, &mcount);
    if (!strcmp(name, "ollama")) {
        char homes[AIDR_MAX_HOMES][PATH_MAX];
        int nh = aidr_homes(homes);
        for (int i = 0; i < nh && mcount < AIDR_MAX_MODELS; i++) {
            char root[PATH_MAX];
            snprintf(root, sizeof(root), "%s/.ollama", homes[i]);
            aidr_ollama_manifests(root, models, &mcount);
        }
        if (mcount < AIDR_MAX_MODELS) {
            aidr_ollama_manifests("/usr/share/ollama/.ollama", models, &mcount);
        }
    }
    gm_aidr_emit(aidr, event);
    cJSON_Delete(event);
}

/* --------------------------------------------------------------- mcp_server */

static void aidr_emit_mcp_process(gm_aidr_t *aidr, const char *pid, const char *comm,
                                  const char *exe) {
    char key[320];
    snprintf(key, sizeof(key), "mcpp:%s:%s", pid, comm);
    if (OSHash_Get(seen_mcp, key)) {
        return;
    }
    OSHash_Add(seen_mcp, key, (void *)1);

    cJSON *event = aidr_new_event("mcp_server");
    cJSON *data = cJSON_AddObjectToObject(event, "data");
    cJSON *proc = cJSON_AddObjectToObject(data, "process");
    cJSON_AddNumberToObject(proc, "pid", atoi(pid));
    cJSON_AddStringToObject(proc, "name", comm);
    if (exe && exe[0]) {
        cJSON_AddStringToObject(proc, "exe", exe);
    }
    cJSON *mcp = cJSON_AddObjectToObject(data, "mcp");
    unsigned int port = aidr_pid_listen_port(pid);
    cJSON_AddStringToObject(mcp, "transport", port ? "http" : "stdio");
    if (port) {
        cJSON_AddNumberToObject(mcp, "listen_port", port);
    }
    gm_aidr_emit(aidr, event);
    cJSON_Delete(event);
}

// Emit MCP servers registered in a client config file ("mcpServers" object of
// { name: {command, args[] | url} }). Real registrations only.
static void aidr_scan_mcp_config(gm_aidr_t *aidr, const char *cfg_path) {
    FILE *f = fopen(cfg_path, "r");
    if (!f) {
        return;
    }
    char *buf = malloc(262144);
    if (!buf) {
        fclose(f);
        return;
    }
    size_t n = fread(buf, 1, 262143, f);
    fclose(f);
    buf[n] = '\0';
    cJSON *root = cJSON_Parse(buf);
    free(buf);
    if (!root) {
        return;
    }
    cJSON *servers = cJSON_GetObjectItem(root, "mcpServers");
    if (!servers) {
        servers = cJSON_GetObjectItem(root, "servers");
    }
    if (servers && cJSON_IsObject(servers)) {
        for (cJSON *srv = servers->child; srv; srv = srv->next) {
            if (!srv->string) continue;
            char key[PATH_MAX + 300];
            snprintf(key, sizeof(key), "mcpc:%s:%s", cfg_path, srv->string);
            if (OSHash_Get(seen_mcp, key)) continue;
            OSHash_Add(seen_mcp, key, (void *)1);

            cJSON *event = aidr_new_event("mcp_server");
            cJSON *data = cJSON_AddObjectToObject(event, "data");
            cJSON *mcp = cJSON_AddObjectToObject(data, "mcp");
            cJSON_AddStringToObject(mcp, "server", srv->string);
            cJSON_AddStringToObject(mcp, "source", cfg_path);
            cJSON *url = cJSON_GetObjectItem(srv, "url");
            cJSON *cmd = cJSON_GetObjectItem(srv, "command");
            if (url && cJSON_IsString(url)) {
                cJSON_AddStringToObject(mcp, "transport",
                                        aidr_stristr(url->valuestring, "/sse") ? "sse" : "http");
                cJSON_AddStringToObject(mcp, "url", url->valuestring);
            } else {
                cJSON_AddStringToObject(mcp, "transport", "stdio");
                if (cmd && cJSON_IsString(cmd)) {
                    cJSON *proc = cJSON_AddObjectToObject(data, "process");
                    cJSON_AddStringToObject(proc, "exe", cmd->valuestring);
                }
            }
            gm_aidr_emit(aidr, event);
            cJSON_Delete(event);
        }
    }
    cJSON_Delete(root);
}

static void aidr_scan_mcp_configs(gm_aidr_t *aidr) {
    static const char *REL[] = {
        ".config/Claude/claude_desktop_config.json",
        ".cursor/mcp.json",
        ".codeium/windsurf/mcp_config.json",
        ".vscode/mcp.json",
        NULL};
    char homes[AIDR_MAX_HOMES][PATH_MAX];
    int nh = aidr_homes(homes);
    for (int i = 0; i < nh; i++) {
        for (int j = 0; REL[j]; j++) {
            char path[PATH_MAX * 2];
            snprintf(path, sizeof(path), "%s/%s", homes[i], REL[j]);
            if (access(path, R_OK) == 0) {
                aidr_scan_mcp_config(aidr, path);
            }
        }
    }
}

/* --------------------------------------------------------------- model_file */

// Derive a source registry/URL from the artifact path structure (real
// provenance only — ollama store or HF hub cache; omitted otherwise).
static void aidr_model_source(const char *path, char *out, size_t out_sz) {
    out[0] = '\0';
    if (strstr(path, "/.ollama/")) {
        snprintf(out, out_sz, "registry.ollama.ai");
        return;
    }
    const char *m = strstr(path, "models--");
    if (m) {
        char org[128] = "", name[128] = "";
        if (sscanf(m, "models--%127[^-/]--%127[^/]", org, name) == 2) {
            snprintf(out, out_sz, "huggingface.co/%s/%s", org, name);
        }
    }
}

static void aidr_emit_model(gm_aidr_t *aidr, const char *path, const char *fmt,
                            long long size) {
    if (OSHash_Get(seen_model, path)) {
        return;
    }
    OSHash_Add(seen_model, path, (void *)1);

    cJSON *event = aidr_new_event("model_file");
    cJSON *data = cJSON_AddObjectToObject(event, "data");
    cJSON *model = cJSON_AddObjectToObject(data, "model");
    cJSON_AddStringToObject(model, "path", path);
    cJSON_AddStringToObject(model, "format", fmt);
    cJSON_AddNumberToObject(model, "size_bytes", (double)size);

    const char *base = strrchr(path, '/');
    base = base ? base + 1 : path;
    const char *blob_sha = aidr_ollama_blob_sha256(base);
    if (blob_sha) {
        // ollama blobs are content-addressed: the digest IS the file name.
        cJSON_AddStringToObject(model, "sha256", blob_sha);
    } else if (aidr->max_hash_mb > 0 && size <= aidr->max_hash_mb * 1024LL * 1024LL) {
        os_sha256 sha = {0};
        if (OS_SHA256_File(path, sha, OS_BINARY) == 0) {
            cJSON_AddStringToObject(model, "sha256", sha);
        }
    }
    char src[300];
    aidr_model_source(path, src, sizeof(src));
    if (src[0]) {
        cJSON_AddStringToObject(model, "source_url", src);
    }
    gm_aidr_emit(aidr, event);
    cJSON_Delete(event);
}

// Is this file a GGUF container? (ollama blobs have no extension — check the
// 4-byte magic instead of guessing.)
static int aidr_is_gguf(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) {
        return 0;
    }
    char magic[4] = {0};
    size_t n = fread(magic, 1, 4, f);
    fclose(f);
    return n == 4 && memcmp(magic, "GGUF", 4) == 0;
}

#define AIDR_MODEL_SWEEP_CAP 200

static void aidr_scan_model_dir(gm_aidr_t *aidr, const char *dir, int depth, int *budget) {
    if (depth > 4 || *budget <= 0) {
        return;
    }
    DIR *d = opendir(dir);
    if (!d) {
        return;
    }
    struct dirent *ent;
    while ((ent = readdir(d)) != NULL && *budget > 0) {
        if (!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, "..")) continue;
        char full[PATH_MAX * 2];
        snprintf(full, sizeof(full), "%s/%s", dir, ent->d_name);
        struct stat st;
        if (lstat(full, &st) != 0) continue;
        if (S_ISDIR(st.st_mode)) {
            aidr_scan_model_dir(aidr, full, depth + 1, budget);
        } else if (S_ISREG(st.st_mode)) {
            const char *fmt = aidr_model_format(ent->d_name);
            if (!fmt && aidr_ollama_blob_sha256(ent->d_name) && st.st_size > 4096 &&
                aidr_is_gguf(full)) {
                fmt = "gguf";
            }
            if (fmt) {
                (*budget)--;
                aidr_emit_model(aidr, full, fmt, (long long)st.st_size);
            }
        }
    }
    closedir(d);
}

static void aidr_scan_models(gm_aidr_t *aidr) {
    static const char *REL[] = {
        ".ollama/models",
        ".cache/huggingface/hub",
        ".cache/lm-studio/models",
        ".lmstudio/models",
        ".local/share/nomic.ai/GPT4All",
        NULL};
    int budget = AIDR_MODEL_SWEEP_CAP;
    char homes[AIDR_MAX_HOMES][PATH_MAX];
    int nh = aidr_homes(homes);
    for (int i = 0; i < nh && budget > 0; i++) {
        for (int j = 0; REL[j] && budget > 0; j++) {
            char path[PATH_MAX * 2];
            snprintf(path, sizeof(path), "%s/%s", homes[i], REL[j]);
            aidr_scan_model_dir(aidr, path, 0, &budget);
        }
    }
    if (budget > 0) {
        aidr_scan_model_dir(aidr, "/usr/share/ollama/.ollama/models", 0, &budget);
    }
}

/* ------------------------------------------------------------- llm_api_call */

static void aidr_emit_api_call(gm_aidr_t *aidr, const char *pid, const char *comm,
                               const char *exe, const aidr_esock_t *es,
                               const aidr_previp_t *prov) {
    char key[160];
    snprintf(key, sizeof(key), "api:%s:%s:%u", pid, es->rip, es->rport);
    if (OSHash_Get(seen_api, key)) {
        return;
    }
    OSHash_Add(seen_api, key, (void *)1);

    cJSON *event = aidr_new_event("llm_api_call");
    cJSON *data = cJSON_AddObjectToObject(event, "data");
    cJSON *proc = cJSON_AddObjectToObject(data, "process");
    cJSON_AddNumberToObject(proc, "pid", atoi(pid));
    cJSON_AddStringToObject(proc, "name", comm);
    if (exe && exe[0]) {
        cJSON_AddStringToObject(proc, "exe", exe);
    }
    cJSON *dest = cJSON_AddObjectToObject(data, "dest");
    cJSON_AddStringToObject(dest, "host", prov->host);
    cJSON_AddStringToObject(dest, "ip", es->rip);
    cJSON_AddNumberToObject(dest, "port", es->rport);
    cJSON_AddStringToObject(dest, "provider", prov->provider);
    gm_aidr_emit(aidr, event);
    cJSON_Delete(event);
}

// Match established connections against the resolved provider-IP table and
// attribute each to its owning process via the socket inode.
static void aidr_scan_api_calls(gm_aidr_t *aidr) {
    // wanted inodes = established sockets whose remote is a known AI endpoint
    unsigned long wanted[64];
    const aidr_esock_t *wanted_es[64];
    const aidr_previp_t *wanted_prov[64];
    int n_wanted = 0;
    for (int i = 0; i < n_esocks && n_wanted < 64; i++) {
        const aidr_previp_t *p = aidr_prov_for_ip(esocks[i].rip);
        if (p) {
            wanted[n_wanted] = esocks[i].inode;
            wanted_es[n_wanted] = &esocks[i];
            wanted_prov[n_wanted] = p;
            n_wanted++;
        }
    }
    if (n_wanted == 0) {
        return;
    }
    // one pass over /proc/<pid>/fd to find the owners
    DIR *d = opendir("/proc");
    if (!d) {
        return;
    }
    struct dirent *ent;
    while ((ent = readdir(d)) != NULL && n_wanted > 0) {
        int numeric = 1;
        for (const char *c = ent->d_name; *c; c++) {
            if (!isdigit((unsigned char)*c)) { numeric = 0; break; }
        }
        if (!numeric || !ent->d_name[0]) continue;
        char fddir[280];
        snprintf(fddir, sizeof(fddir), "/proc/%s/fd", ent->d_name);
        DIR *fd = opendir(fddir);
        if (!fd) continue;
        struct dirent *fent;
        while ((fent = readdir(fd)) != NULL && n_wanted > 0) {
            if (fent->d_name[0] == '.') continue;
            char lpath[560], target[128];
            snprintf(lpath, sizeof(lpath), "%s/%s", fddir, fent->d_name);
            ssize_t n = readlink(lpath, target, sizeof(target) - 1);
            if (n <= 0) continue;
            target[n] = '\0';
            unsigned long inode = 0;
            if (sscanf(target, "socket:[%lu]", &inode) != 1) continue;
            for (int i = 0; i < n_wanted; i++) {
                if (wanted[i] == inode) {
                    char comm[256], cmdline[1024], exe[PATH_MAX];
                    if (aidr_read_proc(ent->d_name, comm, sizeof(comm),
                                       cmdline, sizeof(cmdline), exe, sizeof(exe))) {
                        aidr_emit_api_call(aidr, ent->d_name, comm, exe,
                                           wanted_es[i], wanted_prov[i]);
                    }
                    // remove the matched inode from the wanted set
                    wanted[i] = wanted[n_wanted - 1];
                    wanted_es[i] = wanted_es[n_wanted - 1];
                    wanted_prov[i] = wanted_prov[n_wanted - 1];
                    n_wanted--;
                    break;
                }
            }
        }
        closedir(fd);
    }
    closedir(d);
}

/* ------------------------------------------------------------- process sweep */

static void aidr_scan_processes(gm_aidr_t *aidr) {
    DIR *d = opendir("/proc");
    if (!d) {
        return;
    }
    struct dirent *ent;
    while ((ent = readdir(d)) != NULL) {
        int numeric = 1;
        for (const char *c = ent->d_name; *c; c++) {
            if (!isdigit((unsigned char)*c)) { numeric = 0; break; }
        }
        if (!numeric || !ent->d_name[0]) continue;
        char comm[256], cmdline[4096], exe[PATH_MAX];
        if (!aidr_read_proc(ent->d_name, comm, sizeof(comm),
                            cmdline, sizeof(cmdline), exe, sizeof(exe))) {
            continue;
        }
        if (aidr->flags.runtimes) {
            const char *rt = aidr_classify_runtime(comm, cmdline[0] ? cmdline : exe);
            if (rt) {
                aidr_emit_runtime(aidr, ent->d_name, rt, exe, cmdline);
            }
        }
        if (aidr->flags.mcp && aidr_is_mcp_process(comm, cmdline)) {
            aidr_emit_mcp_process(aidr, ent->d_name, comm, exe);
        }
    }
    closedir(d);
}

/* --------------------------------------------------------------------- main */

static void aidr_bound_caches(void) {
    if (seen_api && OSHash_Get_Elem_ex(seen_api) > 50000) {
        OSHash_Free(seen_api);
        seen_api = OSHash_Create();
    }
    if (seen_model && OSHash_Get_Elem_ex(seen_model) > 20000) {
        OSHash_Free(seen_model);
        seen_model = OSHash_Create();
    }
}

void* gm_aidr_main(gm_aidr_t *aidr) {
    if (!aidr->flags.enabled) {
        mtinfo(GM_AIDR_LOGTAG, "Module disabled. Exiting.");
        pthread_exit(NULL);
    }

    seen_runtime = OSHash_Create();
    seen_api = OSHash_Create();
    seen_mcp = OSHash_Create();
    seen_model = OSHash_Create();

    aidr_log = fopen(GM_AIDR_LOG_PATH, "a");

    if ((queue_fd = StartMQ(DEFAULTQUEUE, WRITE, INFINITE_OPENQ_ATTEMPTS)) < 0) {
        mterror(GM_AIDR_LOGTAG, "Can't connect to queue '%s'.", DEFAULTQUEUE);
    }

    aidr->flags.running = 1;
    mtinfo(GM_AIDR_LOGTAG,
           "Native AIDR telemetry started (interval %us, runtimes=%u api_calls=%u mcp=%u models=%u).",
           aidr->interval, aidr->flags.runtimes, aidr->flags.api_calls,
           aidr->flags.mcp, aidr->flags.models);

    unsigned int cycle = 0;
    while (aidr->flags.running) {
        aidr_refresh_sockets();
        if (aidr->flags.api_calls) {
            aidr_refresh_providers();
            aidr_scan_api_calls(aidr);
        }
        if (aidr->flags.runtimes || aidr->flags.mcp) {
            aidr_scan_processes(aidr);
        }
        if (aidr->flags.mcp) {
            aidr_scan_mcp_configs(aidr);
        }
        // Model artifacts change slowly — sweep every ~6 cycles + first cycle.
        if (aidr->flags.models && (cycle == 0 || cycle % 6 == 0)) {
            aidr_scan_models(aidr);
        }
        aidr_bound_caches();
        cycle++;
        sleep(aidr->interval);
    }

    return NULL;
}

static void gm_aidr_destroy(gm_aidr_t *aidr) {
    if (aidr_log) {
        fclose(aidr_log);
        aidr_log = NULL;
    }
    if (seen_runtime) { OSHash_Free(seen_runtime); seen_runtime = NULL; }
    if (seen_api) { OSHash_Free(seen_api); seen_api = NULL; }
    if (seen_mcp) { OSHash_Free(seen_mcp); seen_mcp = NULL; }
    if (seen_model) { OSHash_Free(seen_model); seen_model = NULL; }
    free(aidr);
}

static cJSON *gm_aidr_dump(const gm_aidr_t *aidr) {
    cJSON *root = cJSON_CreateObject();
    cJSON *gm_aidr = cJSON_CreateObject();
    cJSON_AddStringToObject(gm_aidr, "enabled", aidr->flags.enabled ? "yes" : "no");
    cJSON_AddStringToObject(gm_aidr, "runtimes", aidr->flags.runtimes ? "yes" : "no");
    cJSON_AddStringToObject(gm_aidr, "api_calls", aidr->flags.api_calls ? "yes" : "no");
    cJSON_AddStringToObject(gm_aidr, "mcp", aidr->flags.mcp ? "yes" : "no");
    cJSON_AddStringToObject(gm_aidr, "models", aidr->flags.models ? "yes" : "no");
    cJSON_AddNumberToObject(gm_aidr, "interval", aidr->interval);
    cJSON_AddNumberToObject(gm_aidr, "max_eps", aidr->max_eps);
    cJSON_AddNumberToObject(gm_aidr, "max_hash_mb", aidr->max_hash_mb);
    cJSON_AddItemToObject(root, "aidr", gm_aidr);
    return root;
}

#endif // !WIN32

#ifdef WIN32   /* ---------- Windows agent module (native Win32 telemetry) ---------- */

// Same TU note as wm_edr.c: guardsarm_modulesd_lib does not inherit the win32
// agent target's -D_WIN32_WINNT=0x600; QueryFullProcessImageNameA, inet_ntop
// and getaddrinfo need it. Force it.
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
#include "wm_aidr.h"
#include "wm_aidr_classify.h"
#include "defs.h"
#include "mq_op.h"
#include "cJSON.h"
#include "sha256_op.h"

static void* gm_aidr_main(gm_aidr_t *aidr);
static void gm_aidr_destroy(gm_aidr_t *aidr);
static cJSON *gm_aidr_dump(const gm_aidr_t *aidr);

const gm_context GM_AIDR_CONTEXT = {
    .name = GM_AIDR_CONTEXT_NAME,
    .start = (gm_routine)gm_aidr_main,
    .destroy = (void(*)(void *))gm_aidr_destroy,
    .dump = (cJSON *(*)(const void *))gm_aidr_dump,
    .sync = NULL,
    .stop = NULL,
    .query = NULL,
};

static int queue_fd = 0;
static FILE *aidr_log = NULL;
static OSHash *seen_runtime = NULL;
static OSHash *seen_api = NULL;
static OSHash *seen_mcp = NULL;
static OSHash *seen_model = NULL;

#define AIDR_MAX_PROV_IP 256
#define AIDR_MAX_MODELS  32
#define AIDR_MODEL_SWEEP_CAP 200

typedef struct { char ip[46]; const char *host; const char *provider; } aidr_previp_t;
static aidr_previp_t prov_ips[AIDR_MAX_PROV_IP];
static int n_prov_ips = 0;
static time_t prov_resolved_at = 0;

/* --------------------------------------------------------------------- emit */

static void gm_aidr_emit(gm_aidr_t *aidr, cJSON *event) {
    char *msg = cJSON_PrintUnformatted(event);
    if (!msg) {
        return;
    }
    const int eps = aidr->max_eps > 0 ? (1000000 / (int)aidr->max_eps) : 0;
    if (gm_sendmsg(eps, queue_fd, msg, GM_AIDR_CONTEXT_NAME, LOCALFILE_MQ) < 0) {
        mterror(GM_AIDR_LOGTAG, "Unable to send message to '%s'", DEFAULTQUEUE);
        if ((queue_fd = StartMQ(DEFAULTQUEUE, WRITE, 1)) >= 0) {
            gm_sendmsg(eps, queue_fd, msg, GM_AIDR_CONTEXT_NAME, LOCALFILE_MQ);
        }
    }
    if (aidr_log) {
        fprintf(aidr_log, "%s\n", msg);
        fflush(aidr_log);
    }
    free(msg);
}

static cJSON *aidr_new_event(const char *event_type) {
    cJSON *event = cJSON_CreateObject();
    cJSON_AddStringToObject(event, "collector", "aidr");
    cJSON_AddStringToObject(event, "event_type", event_type);
    return event;
}

/* ---------------------------------------------------------------- providers */

static void aidr_refresh_providers(void) {
    time_t now = time(NULL);
    if (n_prov_ips > 0 && now - prov_resolved_at < 3600) {
        return;
    }
    n_prov_ips = 0;
    for (int i = 0; AIDR_PROVIDER_HOSTS[i].host && n_prov_ips < AIDR_MAX_PROV_IP; i++) {
        struct addrinfo hints, *res = NULL, *rp;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        if (getaddrinfo(AIDR_PROVIDER_HOSTS[i].host, "443", &hints, &res) != 0) {
            continue;
        }
        int per_host = 0;
        for (rp = res; rp && per_host < 4 && n_prov_ips < AIDR_MAX_PROV_IP; rp = rp->ai_next) {
            char ip[46] = "";
            if (rp->ai_family == AF_INET) {
                struct sockaddr_in *sa = (struct sockaddr_in *)rp->ai_addr;
                inet_ntop(AF_INET, &sa->sin_addr, ip, sizeof(ip));
            } else if (rp->ai_family == AF_INET6) {
                struct sockaddr_in6 *sa6 = (struct sockaddr_in6 *)rp->ai_addr;
                inet_ntop(AF_INET6, &sa6->sin6_addr, ip, sizeof(ip));
            }
            if (ip[0]) {
                snprintf(prov_ips[n_prov_ips].ip, sizeof(prov_ips[n_prov_ips].ip), "%s", ip);
                prov_ips[n_prov_ips].host = AIDR_PROVIDER_HOSTS[i].host;
                prov_ips[n_prov_ips].provider = AIDR_PROVIDER_HOSTS[i].provider;
                n_prov_ips++;
                per_host++;
            }
        }
        freeaddrinfo(res);
    }
    prov_resolved_at = now;
}

static const aidr_previp_t *aidr_prov_for_ip(const char *ip) {
    for (int i = 0; i < n_prov_ips; i++) {
        if (!strcmp(prov_ips[i].ip, ip)) {
            return &prov_ips[i];
        }
    }
    return NULL;
}

/* ------------------------------------------------------------------- process */

static void aidr_proc_exe(DWORD pid, char *exe, DWORD exe_sz) {
    exe[0] = '\0';
    HANDLE h = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (h) {
        DWORD sz = exe_sz;
        QueryFullProcessImageNameA(h, 0, exe, &sz);
        CloseHandle(h);
    }
}

// First LISTEN port owned by pid (GetExtendedTcpTable LISTENER view).
static unsigned int aidr_pid_listen_port(DWORD pid) {
    unsigned int port = 0;
    DWORD sz = 0;
    GetExtendedTcpTable(NULL, &sz, FALSE, AF_INET, TCP_TABLE_OWNER_PID_LISTENER, 0);
    if (sz) {
        MIB_TCPTABLE_OWNER_PID *t = (MIB_TCPTABLE_OWNER_PID *)malloc(sz);
        if (t && GetExtendedTcpTable(t, &sz, FALSE, AF_INET, TCP_TABLE_OWNER_PID_LISTENER, 0) == NO_ERROR) {
            for (DWORD i = 0; i < t->dwNumEntries; i++) {
                if (t->table[i].dwOwningPid == pid) {
                    port = ntohs((u_short)t->table[i].dwLocalPort);
                    break;
                }
            }
        }
        free(t);
    }
    return port;
}

/* --------------------------------------------------------------- ai_runtime */

// Collect Ollama model names from <root>\models\manifests\<reg>\<ns>\<model>\<tag>.
static void aidr_ollama_manifests(const char *root, cJSON *models, int *count) {
    char pat[MAX_PATH * 2];
    snprintf(pat, sizeof(pat), "%s\\models\\manifests\\*", root);
    WIN32_FIND_DATAA reg;
    HANDLE hreg = FindFirstFileA(pat, &reg);
    if (hreg == INVALID_HANDLE_VALUE) {
        return;
    }
    do {
        if (reg.cFileName[0] == '.' || !(reg.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) continue;
        char nspat[MAX_PATH * 2];
        snprintf(nspat, sizeof(nspat), "%s\\models\\manifests\\%s\\*", root, reg.cFileName);
        WIN32_FIND_DATAA ns;
        HANDLE hns = FindFirstFileA(nspat, &ns);
        if (hns == INVALID_HANDLE_VALUE) continue;
        do {
            if (ns.cFileName[0] == '.' || !(ns.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) continue;
            char mpat[MAX_PATH * 2];
            snprintf(mpat, sizeof(mpat), "%s\\models\\manifests\\%s\\%s\\*", root, reg.cFileName, ns.cFileName);
            WIN32_FIND_DATAA m;
            HANDLE hm = FindFirstFileA(mpat, &m);
            if (hm == INVALID_HANDLE_VALUE) continue;
            do {
                if (m.cFileName[0] == '.' || !(m.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) continue;
                char tpat[MAX_PATH * 2];
                snprintf(tpat, sizeof(tpat), "%s\\models\\manifests\\%s\\%s\\%s\\*",
                         root, reg.cFileName, ns.cFileName, m.cFileName);
                WIN32_FIND_DATAA t;
                HANDLE ht = FindFirstFileA(tpat, &t);
                if (ht == INVALID_HANDLE_VALUE) continue;
                do {
                    if (t.cFileName[0] == '.') continue;
                    if (*count >= AIDR_MAX_MODELS) break;
                    char name[512];
                    snprintf(name, sizeof(name), "%s:%s", m.cFileName, t.cFileName);
                    cJSON_AddItemToArray(models, cJSON_CreateString(name));
                    (*count)++;
                } while (FindNextFileA(ht, &t));
                FindClose(ht);
            } while (*count < AIDR_MAX_MODELS && FindNextFileA(hm, &m));
            FindClose(hm);
        } while (*count < AIDR_MAX_MODELS && FindNextFileA(hns, &ns));
        FindClose(hns);
    } while (*count < AIDR_MAX_MODELS && FindNextFileA(hreg, &reg));
    FindClose(hreg);
}

// User profile roots to sweep (agent runs as SYSTEM — enumerate C:\Users\*).
#define AIDR_MAX_HOMES 32
static int aidr_homes(char homes[][MAX_PATH]) {
    int n = 0;
    char *sd = getenv("SystemDrive");
    char base[MAX_PATH];
    snprintf(base, sizeof(base), "%s\\Users", sd ? sd : "C:");
    char pat[MAX_PATH + 4];
    snprintf(pat, sizeof(pat), "%s\\*", base);
    WIN32_FIND_DATAA fd;
    HANDLE h = FindFirstFileA(pat, &fd);
    if (h == INVALID_HANDLE_VALUE) {
        return 0;
    }
    do {
        if (fd.cFileName[0] == '.' || !(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) continue;
        if (!_stricmp(fd.cFileName, "Public") || !_stricmp(fd.cFileName, "Default") ||
            !_stricmp(fd.cFileName, "Default User") || !_stricmp(fd.cFileName, "All Users")) continue;
        if (n < AIDR_MAX_HOMES) {
            snprintf(homes[n++], MAX_PATH, "%s\\%s", base, fd.cFileName);
        }
    } while (FindNextFileA(h, &fd));
    FindClose(h);
    return n;
}

static void aidr_emit_runtime(gm_aidr_t *aidr, DWORD pid, const char *name, const char *exe) {
    char key[320];
    snprintf(key, sizeof(key), "rt:%lu:%s", (unsigned long)pid, name);
    if (OSHash_Get(seen_runtime, key)) {
        return;
    }
    OSHash_Add(seen_runtime, key, (void *)1);

    cJSON *event = aidr_new_event("ai_runtime");
    cJSON *data = cJSON_AddObjectToObject(event, "data");
    cJSON *rt = cJSON_AddObjectToObject(data, "runtime");
    cJSON_AddStringToObject(rt, "name", name);
    cJSON_AddNumberToObject(rt, "pid", (double)pid);
    if (exe && exe[0]) {
        cJSON_AddStringToObject(rt, "exe_path", exe);
    }
    unsigned int port = aidr_pid_listen_port(pid);
    if (port) {
        cJSON_AddNumberToObject(rt, "listen_port", port);
    }
    cJSON *models = cJSON_AddArrayToObject(rt, "models_loaded");
    int mcount = 0;
    if (!strcmp(name, "ollama")) {
        char homes[AIDR_MAX_HOMES][MAX_PATH];
        int nh = aidr_homes(homes);
        for (int i = 0; i < nh && mcount < AIDR_MAX_MODELS; i++) {
            char root[MAX_PATH + 16];
            snprintf(root, sizeof(root), "%s\\.ollama", homes[i]);
            aidr_ollama_manifests(root, models, &mcount);
        }
    }
    gm_aidr_emit(aidr, event);
    cJSON_Delete(event);
}

/* --------------------------------------------------------------- mcp_server */

static void aidr_emit_mcp_process(gm_aidr_t *aidr, DWORD pid, const char *name, const char *exe) {
    char key[320];
    snprintf(key, sizeof(key), "mcpp:%lu:%s", (unsigned long)pid, name);
    if (OSHash_Get(seen_mcp, key)) {
        return;
    }
    OSHash_Add(seen_mcp, key, (void *)1);

    cJSON *event = aidr_new_event("mcp_server");
    cJSON *data = cJSON_AddObjectToObject(event, "data");
    cJSON *proc = cJSON_AddObjectToObject(data, "process");
    cJSON_AddNumberToObject(proc, "pid", (double)pid);
    cJSON_AddStringToObject(proc, "name", name);
    if (exe && exe[0]) {
        cJSON_AddStringToObject(proc, "exe", exe);
    }
    cJSON *mcp = cJSON_AddObjectToObject(data, "mcp");
    unsigned int port = aidr_pid_listen_port(pid);
    cJSON_AddStringToObject(mcp, "transport", port ? "http" : "stdio");
    if (port) {
        cJSON_AddNumberToObject(mcp, "listen_port", port);
    }
    gm_aidr_emit(aidr, event);
    cJSON_Delete(event);
}

static void aidr_scan_mcp_config(gm_aidr_t *aidr, const char *cfg_path) {
    FILE *f = fopen(cfg_path, "rb");
    if (!f) {
        return;
    }
    char *buf = malloc(262144);
    if (!buf) {
        fclose(f);
        return;
    }
    size_t n = fread(buf, 1, 262143, f);
    fclose(f);
    buf[n] = '\0';
    cJSON *root = cJSON_Parse(buf);
    free(buf);
    if (!root) {
        return;
    }
    cJSON *servers = cJSON_GetObjectItem(root, "mcpServers");
    if (!servers) {
        servers = cJSON_GetObjectItem(root, "servers");
    }
    if (servers && cJSON_IsObject(servers)) {
        for (cJSON *srv = servers->child; srv; srv = srv->next) {
            if (!srv->string) continue;
            char key[MAX_PATH + 300];
            snprintf(key, sizeof(key), "mcpc:%s:%s", cfg_path, srv->string);
            if (OSHash_Get(seen_mcp, key)) continue;
            OSHash_Add(seen_mcp, key, (void *)1);

            cJSON *event = aidr_new_event("mcp_server");
            cJSON *data = cJSON_AddObjectToObject(event, "data");
            cJSON *mcp = cJSON_AddObjectToObject(data, "mcp");
            cJSON_AddStringToObject(mcp, "server", srv->string);
            cJSON_AddStringToObject(mcp, "source", cfg_path);
            cJSON *url = cJSON_GetObjectItem(srv, "url");
            cJSON *cmd = cJSON_GetObjectItem(srv, "command");
            if (url && cJSON_IsString(url)) {
                cJSON_AddStringToObject(mcp, "transport",
                                        aidr_stristr(url->valuestring, "/sse") ? "sse" : "http");
                cJSON_AddStringToObject(mcp, "url", url->valuestring);
            } else {
                cJSON_AddStringToObject(mcp, "transport", "stdio");
                if (cmd && cJSON_IsString(cmd)) {
                    cJSON *proc = cJSON_AddObjectToObject(data, "process");
                    cJSON_AddStringToObject(proc, "exe", cmd->valuestring);
                }
            }
            gm_aidr_emit(aidr, event);
            cJSON_Delete(event);
        }
    }
    cJSON_Delete(root);
}

static void aidr_scan_mcp_configs(gm_aidr_t *aidr) {
    static const char *REL[] = {
        "AppData\\Roaming\\Claude\\claude_desktop_config.json",
        ".cursor\\mcp.json",
        ".codeium\\windsurf\\mcp_config.json",
        ".vscode\\mcp.json",
        NULL};
    char homes[AIDR_MAX_HOMES][MAX_PATH];
    int nh = aidr_homes(homes);
    for (int i = 0; i < nh; i++) {
        for (int j = 0; REL[j]; j++) {
            char path[MAX_PATH * 2];
            snprintf(path, sizeof(path), "%s\\%s", homes[i], REL[j]);
            if (GetFileAttributesA(path) != INVALID_FILE_ATTRIBUTES) {
                aidr_scan_mcp_config(aidr, path);
            }
        }
    }
}

/* --------------------------------------------------------------- model_file */

static void aidr_model_source(const char *path, char *out, size_t out_sz) {
    out[0] = '\0';
    if (aidr_stristr(path, "\\.ollama\\")) {
        snprintf(out, out_sz, "registry.ollama.ai");
        return;
    }
    const char *m = aidr_stristr(path, "models--");
    if (m) {
        char org[128] = "", name[128] = "";
        if (sscanf(m, "models--%127[^-\\/]--%127[^\\/]", org, name) == 2) {
            snprintf(out, out_sz, "huggingface.co/%s/%s", org, name);
        }
    }
}

static int aidr_is_gguf(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) {
        return 0;
    }
    char magic[4] = {0};
    size_t n = fread(magic, 1, 4, f);
    fclose(f);
    return n == 4 && memcmp(magic, "GGUF", 4) == 0;
}

static void aidr_emit_model(gm_aidr_t *aidr, const char *path, const char *fmt,
                            long long size) {
    if (OSHash_Get(seen_model, path)) {
        return;
    }
    OSHash_Add(seen_model, path, (void *)1);

    cJSON *event = aidr_new_event("model_file");
    cJSON *data = cJSON_AddObjectToObject(event, "data");
    cJSON *model = cJSON_AddObjectToObject(data, "model");
    cJSON_AddStringToObject(model, "path", path);
    cJSON_AddStringToObject(model, "format", fmt);
    cJSON_AddNumberToObject(model, "size_bytes", (double)size);

    const char *base = strrchr(path, '\\');
    base = base ? base + 1 : path;
    const char *blob_sha = aidr_ollama_blob_sha256(base);
    if (blob_sha) {
        cJSON_AddStringToObject(model, "sha256", blob_sha);
    } else if (aidr->max_hash_mb > 0 && size <= aidr->max_hash_mb * 1024LL * 1024LL) {
        os_sha256 sha = {0};
        if (OS_SHA256_File(path, sha, OS_BINARY) == 0) {
            cJSON_AddStringToObject(model, "sha256", sha);
        }
    }
    char src[300];
    aidr_model_source(path, src, sizeof(src));
    if (src[0]) {
        cJSON_AddStringToObject(model, "source_url", src);
    }
    gm_aidr_emit(aidr, event);
    cJSON_Delete(event);
}

static void aidr_scan_model_dir(gm_aidr_t *aidr, const char *dir, int depth, int *budget) {
    if (depth > 4 || *budget <= 0) {
        return;
    }
    char pat[MAX_PATH * 2];
    snprintf(pat, sizeof(pat), "%s\\*", dir);
    WIN32_FIND_DATAA fd;
    HANDLE h = FindFirstFileA(pat, &fd);
    if (h == INVALID_HANDLE_VALUE) {
        return;
    }
    do {
        if (!strcmp(fd.cFileName, ".") || !strcmp(fd.cFileName, "..")) continue;
        char full[MAX_PATH * 2];
        snprintf(full, sizeof(full), "%s\\%s", dir, fd.cFileName);
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            aidr_scan_model_dir(aidr, full, depth + 1, budget);
        } else {
            long long size = ((long long)fd.nFileSizeHigh << 32) | fd.nFileSizeLow;
            const char *fmt = aidr_model_format(fd.cFileName);
            if (!fmt && aidr_ollama_blob_sha256(fd.cFileName) && size > 4096 &&
                aidr_is_gguf(full)) {
                fmt = "gguf";
            }
            if (fmt) {
                (*budget)--;
                aidr_emit_model(aidr, full, fmt, size);
            }
        }
    } while (*budget > 0 && FindNextFileA(h, &fd));
    FindClose(h);
}

static void aidr_scan_models(gm_aidr_t *aidr) {
    static const char *REL[] = {
        ".ollama\\models",
        ".cache\\huggingface\\hub",
        ".cache\\lm-studio\\models",
        ".lmstudio\\models",
        "AppData\\Local\\nomic.ai\\GPT4All",
        NULL};
    int budget = AIDR_MODEL_SWEEP_CAP;
    char homes[AIDR_MAX_HOMES][MAX_PATH];
    int nh = aidr_homes(homes);
    for (int i = 0; i < nh && budget > 0; i++) {
        for (int j = 0; REL[j] && budget > 0; j++) {
            char path[MAX_PATH * 2];
            snprintf(path, sizeof(path), "%s\\%s", homes[i], REL[j]);
            aidr_scan_model_dir(aidr, path, 0, &budget);
        }
    }
}

/* ------------------------------------------------------------- llm_api_call */

static void aidr_emit_api_call(gm_aidr_t *aidr, DWORD pid, const char *rip,
                               unsigned int rport, const aidr_previp_t *prov) {
    char key[160];
    snprintf(key, sizeof(key), "api:%lu:%s:%u", (unsigned long)pid, rip, rport);
    if (OSHash_Get(seen_api, key)) {
        return;
    }
    OSHash_Add(seen_api, key, (void *)1);

    char exe[MAX_PATH] = "";
    aidr_proc_exe(pid, exe, sizeof(exe));
    const char *name = strrchr(exe, '\\');
    name = name ? name + 1 : exe;

    cJSON *event = aidr_new_event("llm_api_call");
    cJSON *data = cJSON_AddObjectToObject(event, "data");
    cJSON *proc = cJSON_AddObjectToObject(data, "process");
    cJSON_AddNumberToObject(proc, "pid", (double)pid);
    cJSON_AddStringToObject(proc, "name", name[0] ? name : "unknown");
    if (exe[0]) {
        cJSON_AddStringToObject(proc, "exe", exe);
    }
    cJSON *dest = cJSON_AddObjectToObject(data, "dest");
    cJSON_AddStringToObject(dest, "host", prov->host);
    cJSON_AddStringToObject(dest, "ip", rip);
    cJSON_AddNumberToObject(dest, "port", rport);
    cJSON_AddStringToObject(dest, "provider", prov->provider);
    gm_aidr_emit(aidr, event);
    cJSON_Delete(event);
}

static void aidr_scan_api_calls(gm_aidr_t *aidr) {
    // IPv4 established connections with owning PID.
    DWORD sz = 0;
    GetExtendedTcpTable(NULL, &sz, FALSE, AF_INET, TCP_TABLE_OWNER_PID_ALL, 0);
    if (sz) {
        MIB_TCPTABLE_OWNER_PID *t = (MIB_TCPTABLE_OWNER_PID *)malloc(sz);
        if (t && GetExtendedTcpTable(t, &sz, FALSE, AF_INET, TCP_TABLE_OWNER_PID_ALL, 0) == NO_ERROR) {
            for (DWORD i = 0; i < t->dwNumEntries; i++) {
                MIB_TCPROW_OWNER_PID *r = &t->table[i];
                if (r->dwState != MIB_TCP_STATE_ESTAB) {
                    continue;
                }
                unsigned char *ra = (unsigned char *)&r->dwRemoteAddr;
                char rip[64];
                snprintf(rip, sizeof(rip), "%u.%u.%u.%u", ra[0], ra[1], ra[2], ra[3]);
                const aidr_previp_t *p = aidr_prov_for_ip(rip);
                if (p) {
                    aidr_emit_api_call(aidr, r->dwOwningPid, rip,
                                       ntohs((u_short)r->dwRemotePort), p);
                }
            }
        }
        free(t);
    }
    // IPv6 established connections with owning PID.
    sz = 0;
    GetExtendedTcpTable(NULL, &sz, FALSE, AF_INET6, TCP_TABLE_OWNER_PID_ALL, 0);
    if (sz) {
        MIB_TCP6TABLE_OWNER_PID *t = (MIB_TCP6TABLE_OWNER_PID *)malloc(sz);
        if (t && GetExtendedTcpTable(t, &sz, FALSE, AF_INET6, TCP_TABLE_OWNER_PID_ALL, 0) == NO_ERROR) {
            for (DWORD i = 0; i < t->dwNumEntries; i++) {
                MIB_TCP6ROW_OWNER_PID *r = &t->table[i];
                if (r->dwState != MIB_TCP_STATE_ESTAB) {
                    continue;
                }
                char rip[64] = "";
                inet_ntop(AF_INET6, r->ucRemoteAddr, rip, sizeof(rip));
                const aidr_previp_t *p = aidr_prov_for_ip(rip);
                if (p) {
                    aidr_emit_api_call(aidr, r->dwOwningPid, rip,
                                       ntohs((u_short)r->dwRemotePort), p);
                }
            }
        }
        free(t);
    }
}

/* ------------------------------------------------------------- process sweep */

static void aidr_scan_processes(gm_aidr_t *aidr) {
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) {
        return;
    }
    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(pe);
    if (Process32First(snap, &pe)) {
        do {
            if (pe.th32ProcessID == 0) continue;
            char exe[MAX_PATH] = "";
            aidr_proc_exe(pe.th32ProcessID, exe, sizeof(exe));
            if (aidr->flags.runtimes) {
                const char *rt = aidr_classify_runtime(pe.szExeFile, exe);
                if (rt) {
                    aidr_emit_runtime(aidr, pe.th32ProcessID, rt, exe);
                }
            }
            if (aidr->flags.mcp && aidr_is_mcp_process(pe.szExeFile, exe)) {
                aidr_emit_mcp_process(aidr, pe.th32ProcessID, pe.szExeFile, exe);
            }
        } while (Process32Next(snap, &pe));
    }
    CloseHandle(snap);
}

/* --------------------------------------------------------------------- main */

static void aidr_bound_caches(void) {
    if (seen_api && OSHash_Get_Elem_ex(seen_api) > 50000) {
        OSHash_Free(seen_api);
        seen_api = OSHash_Create();
    }
    if (seen_model && OSHash_Get_Elem_ex(seen_model) > 20000) {
        OSHash_Free(seen_model);
        seen_model = OSHash_Create();
    }
}

void* gm_aidr_main(gm_aidr_t *aidr) {
    if (!aidr->flags.enabled) {
        mtinfo(GM_AIDR_LOGTAG, "Module disabled. Exiting.");
        return NULL;
    }

    seen_runtime = OSHash_Create();
    seen_api = OSHash_Create();
    seen_mcp = OSHash_Create();
    seen_model = OSHash_Create();

    aidr_log = fopen(GM_AIDR_LOG_PATH, "a");

    if ((queue_fd = StartMQ(DEFAULTQUEUE, WRITE, INFINITE_OPENQ_ATTEMPTS)) < 0) {
        mterror(GM_AIDR_LOGTAG, "Can't connect to queue '%s'.", DEFAULTQUEUE);
    }

    aidr->flags.running = 1;
    mtinfo(GM_AIDR_LOGTAG,
           "Native AIDR telemetry started (interval %us, runtimes=%u api_calls=%u mcp=%u models=%u).",
           aidr->interval, aidr->flags.runtimes, aidr->flags.api_calls,
           aidr->flags.mcp, aidr->flags.models);

    unsigned int cycle = 0;
    while (aidr->flags.running) {
        if (aidr->flags.api_calls) {
            aidr_refresh_providers();
            aidr_scan_api_calls(aidr);
        }
        if (aidr->flags.runtimes || aidr->flags.mcp) {
            aidr_scan_processes(aidr);
        }
        if (aidr->flags.mcp) {
            aidr_scan_mcp_configs(aidr);
        }
        if (aidr->flags.models && (cycle == 0 || cycle % 6 == 0)) {
            aidr_scan_models(aidr);
        }
        aidr_bound_caches();
        cycle++;
        Sleep(aidr->interval * 1000);
    }

    return NULL;
}

static void gm_aidr_destroy(gm_aidr_t *aidr) {
    if (aidr_log) {
        fclose(aidr_log);
        aidr_log = NULL;
    }
    if (seen_runtime) { OSHash_Free(seen_runtime); seen_runtime = NULL; }
    if (seen_api) { OSHash_Free(seen_api); seen_api = NULL; }
    if (seen_mcp) { OSHash_Free(seen_mcp); seen_mcp = NULL; }
    if (seen_model) { OSHash_Free(seen_model); seen_model = NULL; }
    free(aidr);
}

static cJSON *gm_aidr_dump(const gm_aidr_t *aidr) {
    cJSON *root = cJSON_CreateObject();
    cJSON *gm_aidr = cJSON_CreateObject();
    cJSON_AddStringToObject(gm_aidr, "enabled", aidr->flags.enabled ? "yes" : "no");
    cJSON_AddStringToObject(gm_aidr, "runtimes", aidr->flags.runtimes ? "yes" : "no");
    cJSON_AddStringToObject(gm_aidr, "api_calls", aidr->flags.api_calls ? "yes" : "no");
    cJSON_AddStringToObject(gm_aidr, "mcp", aidr->flags.mcp ? "yes" : "no");
    cJSON_AddStringToObject(gm_aidr, "models", aidr->flags.models ? "yes" : "no");
    cJSON_AddNumberToObject(gm_aidr, "interval", aidr->interval);
    cJSON_AddNumberToObject(gm_aidr, "max_eps", aidr->max_eps);
    cJSON_AddNumberToObject(gm_aidr, "max_hash_mb", aidr->max_hash_mb);
    cJSON_AddItemToObject(root, "aidr", gm_aidr);
    return root;
}

#endif // WIN32
