/* Copyright (C) 2026 GuardSarm, Inc.
 * All right reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

/* Active response: isolate — network containment. Allow-lists the manager/cloud
 * so the host stays managed while every other flow is blocked. ENABLE isolates;
 * DISABLE restores.
 *   Linux:   dedicated iptables chains (cleanly reversible, other rules untouched).
 *   Windows: PRIVATE Windows Filtering Platform (WFP) sub-layer + permit/block
 *            filters (see helpers/wfp_isolation.c). This NEVER modifies the global
 *            Windows Firewall policy, the user's rules, routes, or Winsock — so
 *            un-isolation is simply "delete our WFP provider", and an unexpected
 *            reboot auto-clears the runtime filters (fail-open by construction). */

#include "active_responses.h"

#ifdef WIN32
#include "helpers/wfp_isolation.h"
#include "os_xml.h"
#include <time.h>

#define ISO_STATE_FILE "active-response\\isolation.json"
#define ISO_DEFAULT_TIMEOUT 1800   /* 30 min dead-man default */

/* Pull the isolation allow-list + options out of the manager-supplied AR JSON:
 *   guardsarm.active_response.isolation = {
 *       "allow":     ["10.0.0.5","203.0.113.0/24","2001:db8::1"],
 *       "ports":     [1514,1515,443],
 *       "allow_dns": true, "allow_dhcp": true, "timeout": 1800 }
 * Returns the number of allow entries parsed. */
static size_t cfg_from_json(cJSON *root, wfp_isolation_cfg *cfg, unsigned *timeout) {
    memset(cfg, 0, sizeof(*cfg));
    cfg->allow_dns = true;      /* default: let the host still resolve names   */
    cfg->allow_dhcp = true;     /* default: keep the DHCP lease                */
    cfg->allow_icmp = false;
    *timeout = ISO_DEFAULT_TIMEOUT;

    cJSON *gs  = root ? cJSON_GetObjectItem(root, "guardsarm") : NULL;
    cJSON *ar  = gs  ? cJSON_GetObjectItem(gs, "active_response") : NULL;
    cJSON *iso = ar  ? cJSON_GetObjectItem(ar, "isolation") : NULL;
    if (!iso) return 0;

    cJSON *allow = cJSON_GetObjectItem(iso, "allow");
    if (allow && cJSON_IsArray(allow)) {
        cJSON *it = NULL;
        cJSON_ArrayForEach(it, allow) {
            if (cfg->allow_count >= WFP_ISO_MAX_ALLOW) break;
            if (cJSON_IsString(it) && it->valuestring &&
                wfp_parse_allow(it->valuestring, &cfg->allow[cfg->allow_count]) == 0) {
                cfg->allow_count++;
            }
        }
    }
    cJSON *ports = cJSON_GetObjectItem(iso, "ports");
    if (ports && cJSON_IsArray(ports)) {
        cJSON *it = NULL;
        cJSON_ArrayForEach(it, ports) {
            if (cfg->port_count >= WFP_ISO_MAX_PORTS) break;
            if (cJSON_IsNumber(it) && it->valueint > 0 && it->valueint <= 65535)
                cfg->ports[cfg->port_count++] = it->valueint;
        }
    }
    cJSON *dns = cJSON_GetObjectItem(iso, "allow_dns");
    if (cJSON_IsBool(dns))  cfg->allow_dns  = cJSON_IsTrue(dns);
    cJSON *dhcp = cJSON_GetObjectItem(iso, "allow_dhcp");
    if (cJSON_IsBool(dhcp)) cfg->allow_dhcp = cJSON_IsTrue(dhcp);
    cJSON *tmo = cJSON_GetObjectItem(iso, "timeout");
    if (cJSON_IsNumber(tmo) && tmo->valueint > 0) *timeout = (unsigned)tmo->valueint;

    return cfg->allow_count;
}

/* Fallback allow-list source when the manager did not inject one: read the agent's
 * own manager address(es) from the local config and resolve to IP(s). Best-effort;
 * tries the known config filenames + root element names across rebrand states. */
static void cfg_from_config(wfp_isolation_cfg *cfg) {
    const char *files[] = { "etc/gsmsec.conf", "gsmsec.conf", "etc/ossec.conf", "ossec.conf", NULL };
    const char *roots[] = { "guardsarm_config", "ossec_config", NULL };
    for (int fi = 0; files[fi] && cfg->allow_count == 0; fi++) {
        OS_XML xml;
        if (OS_ReadXML(files[fi], &xml) < 0) continue;
        for (int ri = 0; roots[ri] && cfg->allow_count < WFP_ISO_MAX_ALLOW; ri++) {
            const char *path[] = { roots[ri], "client", "server", "address", NULL };
            char **vals = OS_GetContents(&xml, (const char **)path);   /* <address> contents */
            if (!vals) continue;
            for (int i = 0; vals[i] && cfg->allow_count < WFP_ISO_MAX_ALLOW; i++) {
                /* literal IP? add directly; else resolve the hostname */
                if (wfp_parse_allow(vals[i], &cfg->allow[cfg->allow_count]) == 0) {
                    cfg->allow_count++;
                } else {
                    struct addrinfo hints, *res = NULL;
                    memset(&hints, 0, sizeof(hints));
                    hints.ai_family = AF_UNSPEC;
                    if (getaddrinfo(vals[i], NULL, &hints, &res) == 0) {
                        for (struct addrinfo *p = res; p && cfg->allow_count < WFP_ISO_MAX_ALLOW; p = p->ai_next) {
                            wfp_allow_entry *e = &cfg->allow[cfg->allow_count];
                            memset(e, 0, sizeof(*e));
                            if (p->ai_family == AF_INET) {
                                e->family = AF_INET; e->prefix = 32;
                                memcpy(e->addr, &((struct sockaddr_in *)p->ai_addr)->sin_addr, 4);
                                cfg->allow_count++;
                            } else if (p->ai_family == AF_INET6) {
                                e->family = AF_INET6; e->prefix = 128;
                                memcpy(e->addr, &((struct sockaddr_in6 *)p->ai_addr)->sin6_addr, 16);
                                cfg->allow_count++;
                            }
                        }
                        if (res) freeaddrinfo(res);
                    }
                }
            }
            for (int i = 0; vals[i]; i++) os_free(vals[i]);
            os_free(vals);
        }
        OS_ClearXML(&xml);
    }
}

/* Persist isolation state so the recovery watchdog can enforce the dead-man
 * timeout and fail open if the agent dies while isolated. */
static void write_state(const wfp_isolation_cfg *cfg, unsigned timeout, int active) {
    FILE *fp = fopen(ISO_STATE_FILE, "w");
    if (!fp) return;
    if (active) {
        fprintf(fp, "{\"active\":true,\"epoch\":%lld,\"timeout\":%u,\"allow_count\":%zu}\n",
                (long long)time(NULL), timeout, cfg ? cfg->allow_count : 0);
    } else {
        fprintf(fp, "{\"active\":false,\"epoch\":%lld}\n", (long long)time(NULL));
    }
    fclose(fp);
}
#endif /* WIN32 */

#ifndef WIN32
// Run iptables with the given NULL-terminated args; returns 0 on spawn success.
static int ipt(const char *bin, char *const args[]) {
    wfd_t *wfd = wpopenv(bin, (char **)args, W_BIND_STDERR);
    if (!wfd) return -1;
    wpclose(wfd);
    return 0;
}
#endif

int main(int argc, char **argv) {
    (void)argc;
    int action = OS_INVALID;
    cJSON *input_json = NULL;

    action = setup_and_check_message(argv, &input_json);
    if ((action != ENABLE_COMMAND) && (action != DISABLE_COMMAND)) {
        return OS_INVALID;
    }

#ifdef WIN32
    /* Windows: WFP private-sub-layer isolation. NEVER touches global firewall
     * policy — the exact failure mode that previously stranded endpoints. */
    wfp_report rep;

    if (action == ENABLE_COMMAND) {
        wfp_isolation_cfg cfg;
        unsigned timeout = ISO_DEFAULT_TIMEOUT;

        /* 1) manager-injected allow-list (authoritative) */
        cfg_from_json(input_json, &cfg, &timeout);
        /* 2) fallback: derive the manager IP(s) from local config */
        if (cfg.allow_count == 0) {
            write_debug_file(argv[0], "no allow-list in AR message; deriving manager address from local config");
            cfg_from_config(&cfg);
        }
        /* 3) default manager ports if none specified */
        if (cfg.port_count == 0) {
            cfg.ports[0] = 1514; cfg.ports[1] = 1515; cfg.ports[2] = 443; cfg.port_count = 3;
        }

        if (cfg.allow_count == 0) {
            /* Refuse to isolate with an empty allow-list — that would sever the
             * manager link with no way back in-band. Fail SAFE (do nothing). */
            write_debug_file(argv[0], "REFUSING to isolate: could not determine any manager/allow-list address");
            cJSON_Delete(input_json);
            return OS_INVALID;
        }

        if (wfp_isolate(&cfg, &rep) == 0) {
            write_state(&cfg, timeout, 1);
            write_debug_file(argv[0], rep.detail);
            write_debug_file(argv[0], "Isolated (WFP private sub-layer; global firewall policy untouched)");
        } else {
            write_debug_file(argv[0], rep.detail);
            write_debug_file(argv[0], "Isolation FAILED — transaction rolled back, no filters applied");
            cJSON_Delete(input_json);
            return OS_INVALID;
        }
    } else { /* DISABLE_COMMAND */
        if (wfp_unisolate(&rep) == 0) {
            write_state(NULL, 0, 0);
            write_debug_file(argv[0], rep.detail);
            write_debug_file(argv[0], "Un-isolated (removed GuardsArm WFP filters/sub-layer/provider)");
        } else {
            write_debug_file(argv[0], rep.detail);
            write_debug_file(argv[0], "Un-isolation reported an engine error (see detail)");
        }
    }
#else
    char *bin = NULL;
    if (get_binary_path("iptables", &bin) < 0) {
        write_debug_file(argv[0], "iptables not found on this system");
        cJSON_Delete(input_json); os_free(bin); return OS_SUCCESS;
    }

    if (action == ENABLE_COMMAND) {
        // Build the containment chains (idempotent).
        char *c_new_in[]  = { bin, "-N", "GS_ISOLATE_IN", NULL };  ipt(bin, c_new_in);
        char *c_new_out[] = { bin, "-N", "GS_ISOLATE_OUT", NULL }; ipt(bin, c_new_out);
        char *c_f_in[]  = { bin, "-F", "GS_ISOLATE_IN", NULL };  ipt(bin, c_f_in);
        char *c_f_out[] = { bin, "-F", "GS_ISOLATE_OUT", NULL }; ipt(bin, c_f_out);
        char *in_lo[]  = { bin, "-A", "GS_ISOLATE_IN", "-i", "lo", "-j", "ACCEPT", NULL }; ipt(bin, in_lo);
        char *out_lo[] = { bin, "-A", "GS_ISOLATE_OUT", "-o", "lo", "-j", "ACCEPT", NULL }; ipt(bin, out_lo);
        char *in_est[]  = { bin, "-A", "GS_ISOLATE_IN", "-m", "conntrack", "--ctstate", "ESTABLISHED,RELATED", "-j", "ACCEPT", NULL }; ipt(bin, in_est);
        char *out_est[] = { bin, "-A", "GS_ISOLATE_OUT", "-m", "conntrack", "--ctstate", "ESTABLISHED,RELATED", "-j", "ACCEPT", NULL }; ipt(bin, out_est);
        char *in_drop[]  = { bin, "-A", "GS_ISOLATE_IN", "-j", "DROP", NULL };  ipt(bin, in_drop);
        char *out_drop[] = { bin, "-A", "GS_ISOLATE_OUT", "-j", "DROP", NULL }; ipt(bin, out_drop);
        // Jump INPUT/OUTPUT into our chains (first).
        char *j_in[]  = { bin, "-I", "INPUT", "1", "-j", "GS_ISOLATE_IN", NULL };  ipt(bin, j_in);
        char *j_out[] = { bin, "-I", "OUTPUT", "1", "-j", "GS_ISOLATE_OUT", NULL }; ipt(bin, j_out);
        write_debug_file(argv[0], "Isolated endpoint (loopback + established preserved)");
    } else {
        // Remove the jumps and tear down the chains.
        char *d_in[]  = { bin, "-D", "INPUT", "-j", "GS_ISOLATE_IN", NULL };  ipt(bin, d_in);
        char *d_out[] = { bin, "-D", "OUTPUT", "-j", "GS_ISOLATE_OUT", NULL }; ipt(bin, d_out);
        char *f_in[]  = { bin, "-F", "GS_ISOLATE_IN", NULL };  ipt(bin, f_in);
        char *f_out[] = { bin, "-F", "GS_ISOLATE_OUT", NULL }; ipt(bin, f_out);
        char *x_in[]  = { bin, "-X", "GS_ISOLATE_IN", NULL };  ipt(bin, x_in);
        char *x_out[] = { bin, "-X", "GS_ISOLATE_OUT", NULL }; ipt(bin, x_out);
        write_debug_file(argv[0], "Un-isolated endpoint");
    }
    os_free(bin);
#endif

    write_debug_file(argv[0], "Ended");
    cJSON_Delete(input_json);
    return OS_SUCCESS;
}
