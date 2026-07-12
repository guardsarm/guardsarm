/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying,
 * distribution, modification, or use is prohibited except under a written license
 * agreement with GuardSarm, Inc.
 */

/* GuardsArm Endpoint Isolation — Windows Filtering Platform (WFP) engine.
 *
 * Enterprise-grade host containment that NEVER modifies Windows global firewall
 * policy, the user's firewall rules, routing table, Winsock catalog, or adapter
 * bindings. Isolation is implemented as permit/block filters installed inside a
 * PRIVATE GuardsArm WFP sub-layer, tagged to a dedicated GuardsArm provider.
 *
 * Safety model:
 *   - We only ever ADD objects under our own provider GUID. Un-isolation removes
 *     exactly those objects, restoring the prior state perfectly (nothing else was
 *     ever touched).
 *   - Enforcement filters are RUNTIME (non-persistent) objects: they survive the
 *     short-lived isolate.exe exiting, but the Base Filtering Engine auto-clears
 *     them on the next reboot. An unexpected shutdown therefore FAILS OPEN.
 *   - Every apply/remove runs inside a WFP transaction: any failure aborts the
 *     whole change (no half-applied isolation).
 *
 * This header is safe to include on non-Windows (everything compiles out).
 */

#ifndef WFP_ISOLATION_H
#define WFP_ISOLATION_H

#ifdef WIN32

#include <stddef.h>
#include <stdbool.h>

#define WFP_ISO_MAX_ALLOW   64      /* max allow-list entries (IPs/CIDRs)       */
#define WFP_ISO_MAX_PORTS   16      /* max explicitly-permitted manager ports   */
#define WFP_REPORT_LEN      4096    /* detail buffer size for reports           */

/* One allow-list entry: an IPv4/IPv6 host or CIDR the isolated endpoint may still
 * reach (manager, cloud gateway, failover manager, ...). */
typedef struct wfp_allow_entry {
    int           family;    /* AF_INET (2) or AF_INET6 (23)                    */
    unsigned char addr[16];  /* network-order address bytes (4 used for v4)     */
    int           prefix;    /* CIDR prefix length; 32/128 == single host       */
} wfp_allow_entry;

/* Isolation configuration. All fields optional except at least one allow entry
 * (otherwise the endpoint could never reach its manager). */
typedef struct wfp_isolation_cfg {
    wfp_allow_entry allow[WFP_ISO_MAX_ALLOW];
    size_t          allow_count;

    int             ports[WFP_ISO_MAX_PORTS];  /* manager ports always permitted */
    size_t          port_count;                /* e.g. 1514,1515,443             */

    bool            allow_dns;    /* permit UDP/TCP remote port 53              */
    bool            allow_dhcp;   /* permit UDP 67/68 (keep the DHCP lease)     */
    bool            allow_icmp;   /* permit ICMP to allow-listed hosts only     */

    unsigned        timeout_secs; /* dead-man lifetime; 0 == engine default     */
} wfp_isolation_cfg;

/* Structured result for logging / the repair report. */
typedef struct wfp_report {
    bool ok;
    int  filters_added;
    int  filters_removed;
    char detail[WFP_REPORT_LEN];
} wfp_report;

/* Apply isolation. Creates the GuardsArm provider + sub-layer (if absent) and
 * installs permit filters for the allow-list + a block-all filter, all inside a
 * single WFP transaction. Idempotent: re-applying refreshes the filter set.
 * Returns 0 on success, negative on failure (nothing applied on failure). */
int  wfp_isolate(const wfp_isolation_cfg *cfg, wfp_report *rep);

/* Remove ALL GuardsArm WFP objects (filters -> sub-layer -> provider), in a
 * transaction. Idempotent and always safe to call, even if not isolated.
 * Returns 0 on success (or nothing-to-do), negative on engine error. */
int  wfp_unisolate(wfp_report *rep);

/* True iff the GuardsArm sub-layer/provider currently exists in the engine. */
bool wfp_is_isolated(void);

/* Full network repair (used by `repair-network` and the recovery watchdog):
 *   1. wfp_unisolate() — remove every GuardsArm filter/sub-layer/provider.
 *   2. Flush the DNS resolver cache.
 *   3. (optional) queue a Winsock catalog reset — only if reset_winsock is set;
 *      this needs a reboot to take effect and is a defensive extra (normal
 *      isolation never touches Winsock, so this is rarely needed).
 *   4. Verify connectivity (best-effort TCP probe to an allow-listed / public
 *      endpoint) and record the outcome in rep->detail.
 * Returns 0 if isolation objects were removed (or none present) and no hard
 * engine error occurred; negative on engine failure. */
int  wfp_repair_network(bool reset_winsock, wfp_report *rep);

/* Parse a textual address or CIDR ("203.0.113.10", "10.0.0.0/24", "2001:db8::1")
 * into an allow entry. Returns 0 on success, negative on parse failure. */
int  wfp_parse_allow(const char *text, wfp_allow_entry *out);

/* Non-destructive engine self-test (safe to run on any host, incl. production):
 * creates the GuardsArm provider + sub-layer and a handful of PERMIT-ONLY filters
 * (NO block-all, so it cannot isolate anything), verifies they were added and are
 * enumerable, then removes everything and confirms cleanup. Exercises the full
 * add -> enumerate -> delete lifecycle + transactions + every condition type.
 * Returns 0 if the engine round-trips correctly, negative otherwise. Requires
 * admin/SYSTEM (WFP write access). */
int  wfp_selftest(wfp_report *rep);

#endif /* WIN32 */
#endif /* WFP_ISOLATION_H */
