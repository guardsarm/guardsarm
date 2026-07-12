/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying,
 * distribution, modification, or use is prohibited except under a written license
 * agreement with GuardSarm, Inc.
 */

/* GuardsArm Endpoint Isolation — WFP engine implementation. See wfp_isolation.h
 * for the safety model. In one sentence: isolation lives entirely inside a PRIVATE
 * GuardsArm WFP provider + sub-layer; we never touch global firewall policy, the
 * user's rules, routes, Winsock, or adapter bindings, so recovery is simply
 * "delete our provider". */

/* The WFP management API (FWPM_SESSION0/FWPM_FILTER0, FWPM_LAYER_ALE_* GUIDs, ...)
 * is declared in <fwpmu.h> only when the Windows headers target Vista or later.
 * mingw defaults can leave _WIN32_WINNT below 0x0600, which #if-guards the entire
 * WFP surface out ("FWPM_SESSION0 undeclared"). Force Windows 7 (0x0601) BEFORE any
 * Windows header is pulled in, so the WFP declarations are always present. */
#ifdef WIN32
#undef  _WIN32_WINNT
#define _WIN32_WINNT  0x0601
#undef  WINVER
#define WINVER        0x0601
#undef  NTDDI_VERSION
#define NTDDI_VERSION 0x06010000   /* NTDDI_WIN7 */
#endif

#include "wfp_isolation.h"

#ifdef WIN32

/* INITGUID must precede the WFP headers so DEFINE_GUID actually allocates the
 * FWPM_LAYER_* / FWPM_CONDITION_* GUIDs into this translation unit — that way we
 * do not depend on a toolchain-provided GUID import lib (mingw's is patchy). The
 * definitions are __declspec(selectany), so duplicates across TUs merge safely. */
#define INITGUID

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <winsock2.h>
#include <ws2ipdef.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <initguid.h>
#include <fwpmu.h>
#include <fwpmtypes.h>
#include <stdio.h>
#include <string.h>

/* ---- GuardsArm private WFP object identity (fixed, well-known) --------------- */
/* Provider {7B3D9A10-1CE4-4E7B-9C21-0A5F2E6D0011} */
DEFINE_GUID(GUARDSARM_WFP_PROVIDER,
    0x7b3d9a10, 0x1ce4, 0x4e7b, 0x9c, 0x21, 0x0a, 0x5f, 0x2e, 0x6d, 0x00, 0x11);
/* Sub-layer {7B3D9A11-1CE4-4E7B-9C21-0A5F2E6D0011} */
DEFINE_GUID(GUARDSARM_WFP_SUBLAYER,
    0x7b3d9a11, 0x1ce4, 0x4e7b, 0x9c, 0x21, 0x0a, 0x5f, 0x2e, 0x6d, 0x00, 0x11);

/* mingw-w64's WFP headers ship the types + functions but NOT the built-in layer/
 * condition GUID constants nor the FWP_E_* status codes. Define them here with the
 * exact values from the Windows SDK (um/fwpmu.h + shared/winerror.h). INITGUID (set
 * above) makes DEFINE_GUID emit real symbols, so no GUID import lib is needed. */
#ifndef FWP_E_PROVIDER_NOT_FOUND
#define FWP_E_PROVIDER_NOT_FOUND  0x80320005
#endif
#ifndef FWP_E_SUBLAYER_NOT_FOUND
#define FWP_E_SUBLAYER_NOT_FOUND  0x80320007
#endif
#ifndef FWP_E_ALREADY_EXISTS
#define FWP_E_ALREADY_EXISTS      0x80320009
#endif

DEFINE_GUID(FWPM_LAYER_ALE_AUTH_CONNECT_V4,
    0xc38d57d1, 0x05a7, 0x4c33, 0x90, 0x4f, 0x7f, 0xbc, 0xee, 0xe6, 0x0e, 0x82);
DEFINE_GUID(FWPM_LAYER_ALE_AUTH_CONNECT_V6,
    0x4a72393b, 0x319f, 0x44bc, 0x84, 0xc3, 0xba, 0x54, 0xdc, 0xb3, 0xb6, 0xb4);
DEFINE_GUID(FWPM_LAYER_ALE_AUTH_RECV_ACCEPT_V4,
    0xe1cd9fe7, 0xf4b5, 0x4273, 0x96, 0xc0, 0x59, 0x2e, 0x48, 0x7b, 0x86, 0x50);
DEFINE_GUID(FWPM_LAYER_ALE_AUTH_RECV_ACCEPT_V6,
    0xa3b42c97, 0x9f04, 0x4672, 0xb8, 0x7e, 0xce, 0xe9, 0xc4, 0x83, 0x25, 0x7f);
DEFINE_GUID(FWPM_CONDITION_FLAGS,
    0x632ce23b, 0x5167, 0x435c, 0x86, 0xd7, 0xe9, 0x03, 0x68, 0x4a, 0xa8, 0x0c);
DEFINE_GUID(FWPM_CONDITION_IP_REMOTE_ADDRESS,
    0xb235ae9a, 0x1d64, 0x49b8, 0xa4, 0x4c, 0x5f, 0xf3, 0xd9, 0x09, 0x50, 0x45);
DEFINE_GUID(FWPM_CONDITION_IP_PROTOCOL,
    0x3971ef2b, 0x623e, 0x4f9a, 0x8c, 0xb1, 0x6e, 0x79, 0xb8, 0x06, 0xb9, 0xa7);
DEFINE_GUID(FWPM_CONDITION_IP_LOCAL_PORT,
    0x0c1ba1af, 0x5765, 0x453f, 0xaf, 0x22, 0xa8, 0xf7, 0x91, 0xac, 0x77, 0x5b);
DEFINE_GUID(FWPM_CONDITION_IP_REMOTE_PORT,
    0xc35a604d, 0xd22b, 0x4e1a, 0x91, 0xb4, 0x68, 0xf6, 0x74, 0xee, 0x67, 0x4b);

static const wchar_t *WFP_PROVIDER_NAME = L"GuardsArm";
static const wchar_t *WFP_PROVIDER_DESC = L"GuardsArm Endpoint Isolation";
static const wchar_t *WFP_SUBLAYER_NAME = L"GuardsArm Endpoint Isolation";
static const wchar_t *WFP_SUBLAYER_DESC = L"Temporary Endpoint Isolation";
static const wchar_t *WFP_FILTER_DESC   = L"Temporary Endpoint Isolation";

/* Filter weights within our sub-layer: higher wins. Permits must beat block-all. */
#define WFP_WEIGHT_BLOCK   0x00
#define WFP_WEIGHT_PERMIT  0x0E
/* Our sub-layer's cross-sub-layer weight (does not affect block-precedence, only
 * ordering; a block in our sub-layer is authoritative regardless). */
#define WFP_SUBLAYER_WEIGHT 0x8000

/* The four ALE authorization layers we enforce at (outbound + inbound, v4 + v6). */
static const GUID *const WFP_ENFORCE_LAYERS[4] = {
    &FWPM_LAYER_ALE_AUTH_CONNECT_V4,
    &FWPM_LAYER_ALE_AUTH_CONNECT_V6,
    &FWPM_LAYER_ALE_AUTH_RECV_ACCEPT_V4,
    &FWPM_LAYER_ALE_AUTH_RECV_ACCEPT_V6,
};
#define WFP_LAYER_CONNECT_V4 0
#define WFP_LAYER_CONNECT_V6 1
#define WFP_LAYER_ACCEPT_V4  2
#define WFP_LAYER_ACCEPT_V6  3

/* --------------------------------------------------------------------------- */
/* Small report helpers                                                        */
/* --------------------------------------------------------------------------- */
static void rep_reset(wfp_report *rep) {
    if (rep) { rep->ok = false; rep->filters_added = 0; rep->filters_removed = 0; rep->detail[0] = '\0'; }
}
static void rep_add(wfp_report *rep, const char *fmt, ...) {
    if (!rep) return;
    size_t used = strnlen(rep->detail, WFP_REPORT_LEN);
    if (used >= WFP_REPORT_LEN - 1) return;
    va_list ap; va_start(ap, fmt);
    vsnprintf(rep->detail + used, WFP_REPORT_LEN - used, fmt, ap);
    va_end(ap);
}

/* --------------------------------------------------------------------------- */
/* Engine lifecycle                                                            */
/* --------------------------------------------------------------------------- */
static DWORD engine_open(HANDLE *engine) {
    FWPM_SESSION0 session;
    memset(&session, 0, sizeof(session));
    /* NOT FWPM_SESSION_FLAG_DYNAMIC: dynamic objects vanish when this short-lived
     * process exits — we want the filters to persist past isolate.exe. NOT
     * persistent either — runtime objects are auto-cleared by the BFE on reboot
     * (fail-open on unexpected shutdown). */
    session.displayData.name = (wchar_t *)WFP_PROVIDER_NAME;
    session.txnWaitTimeoutInMSec = 5000;
    return FwpmEngineOpen0(NULL, RPC_C_AUTHN_WINNT, NULL, &session, engine);
}

/* Ensure the GuardsArm provider + sub-layer exist. Idempotent (already-exists is
 * treated as success). Must be called inside an open transaction. */
static DWORD ensure_identity(HANDLE engine) {
    DWORD st;

    FWPM_PROVIDER0 provider;
    memset(&provider, 0, sizeof(provider));
    provider.providerKey       = GUARDSARM_WFP_PROVIDER;
    provider.displayData.name  = (wchar_t *)WFP_PROVIDER_NAME;
    provider.displayData.description = (wchar_t *)WFP_PROVIDER_DESC;
    /* runtime (non-persistent) — no FWPM_PROVIDER_FLAG_PERSISTENT */
    st = FwpmProviderAdd0(engine, &provider, NULL);
    if (st != ERROR_SUCCESS && st != FWP_E_ALREADY_EXISTS) return st;

    FWPM_SUBLAYER0 sublayer;
    memset(&sublayer, 0, sizeof(sublayer));
    sublayer.subLayerKey            = GUARDSARM_WFP_SUBLAYER;
    sublayer.displayData.name       = (wchar_t *)WFP_SUBLAYER_NAME;
    sublayer.displayData.description = (wchar_t *)WFP_SUBLAYER_DESC;
    sublayer.providerKey            = (GUID *)&GUARDSARM_WFP_PROVIDER;
    sublayer.weight                 = WFP_SUBLAYER_WEIGHT;
    st = FwpmSubLayerAdd0(engine, &sublayer, NULL);
    if (st != ERROR_SUCCESS && st != FWP_E_ALREADY_EXISTS) return st;

    return ERROR_SUCCESS;
}

/* --------------------------------------------------------------------------- */
/* Filter construction                                                         */
/* --------------------------------------------------------------------------- */

/* Base initializer shared by every filter we add: our provider, our sub-layer,
 * runtime object, tagged display data. */
static void filter_base(FWPM_FILTER0 *f, const GUID *layer, const wchar_t *name) {
    memset(f, 0, sizeof(*f));
    f->layerKey                = *layer;
    f->subLayerKey             = GUARDSARM_WFP_SUBLAYER;
    f->providerKey             = (GUID *)&GUARDSARM_WFP_PROVIDER;
    f->displayData.name        = (wchar_t *)name;
    f->displayData.description = (wchar_t *)WFP_FILTER_DESC;
    f->weight.type             = FWP_EMPTY;   /* set by caller */
    /* flags = 0 -> runtime, non-persistent */
}

/* Block-everything filter (lowest weight) for one layer. */
static DWORD add_block_all(HANDLE engine, const GUID *layer, int *added) {
    FWPM_FILTER0 f;
    filter_base(&f, layer, L"GuardsArm Endpoint Isolation");
    f.weight.type       = FWP_UINT8;
    f.weight.uint8      = WFP_WEIGHT_BLOCK;
    f.numFilterConditions = 0;              /* match all */
    f.action.type       = FWP_ACTION_BLOCK;
    DWORD st = FwpmFilterAdd0(engine, &f, NULL, NULL);
    if (st == ERROR_SUCCESS && added) (*added)++;
    return st;
}

/* Permit filter with an arbitrary condition set (higher weight than block-all). */
static DWORD add_permit(HANDLE engine, const GUID *layer,
                        FWPM_FILTER_CONDITION0 *conds, UINT32 n, int *added) {
    FWPM_FILTER0 f;
    filter_base(&f, layer, L"GuardsArm Endpoint Isolation");
    f.weight.type         = FWP_UINT8;
    f.weight.uint8        = WFP_WEIGHT_PERMIT;
    f.numFilterConditions = n;
    f.filterCondition     = conds;
    f.action.type         = FWP_ACTION_PERMIT;
    DWORD st = FwpmFilterAdd0(engine, &f, NULL, NULL);
    if (st == ERROR_SUCCESS && added) (*added)++;
    return st;
}

/* Permit loopback on a layer (127.0.0.0/8, ::1, and any interface-local flow). */
static DWORD add_permit_loopback(HANDLE engine, const GUID *layer, int *added) {
    FWPM_FILTER_CONDITION0 c;
    memset(&c, 0, sizeof(c));
    c.fieldKey                 = FWPM_CONDITION_FLAGS;
    c.matchType                = FWP_MATCH_FLAGS_ANY_SET;
    c.conditionValue.type      = FWP_UINT32;
    c.conditionValue.uint32    = FWP_CONDITION_FLAG_IS_LOOPBACK;
    return add_permit(engine, layer, &c, 1, added);
}

/* Permit traffic to/from a single allow-list host or CIDR on a v4 layer. */
static DWORD add_permit_addr_v4(HANDLE engine, const GUID *layer,
                                const wfp_allow_entry *e, int *added) {
    UINT32 host = ((UINT32)e->addr[0] << 24) | ((UINT32)e->addr[1] << 16) |
                  ((UINT32)e->addr[2] << 8)  |  (UINT32)e->addr[3];
    FWPM_FILTER_CONDITION0 c;
    memset(&c, 0, sizeof(c));
    c.fieldKey  = FWPM_CONDITION_IP_REMOTE_ADDRESS;
    c.matchType = FWP_MATCH_EQUAL;
    if (e->prefix >= 32) {
        c.conditionValue.type   = FWP_UINT32;
        c.conditionValue.uint32 = host;
    } else {
        static FWP_V4_ADDR_AND_MASK am;   /* static: lives past this call for FwpmFilterAdd */
        am.addr = host;
        am.mask = (e->prefix == 0) ? 0 : (0xFFFFFFFFu << (32 - e->prefix));
        c.conditionValue.type     = FWP_V4_ADDR_MASK;
        c.conditionValue.v4AddrMask = &am;
    }
    return add_permit(engine, layer, &c, 1, added);
}

static DWORD add_permit_addr_v6(HANDLE engine, const GUID *layer,
                                const wfp_allow_entry *e, int *added) {
    FWPM_FILTER_CONDITION0 c;
    memset(&c, 0, sizeof(c));
    c.fieldKey  = FWPM_CONDITION_IP_REMOTE_ADDRESS;
    c.matchType = FWP_MATCH_EQUAL;
    if (e->prefix >= 128) {
        static FWP_BYTE_ARRAY16 b16;
        memcpy(b16.byteArray16, e->addr, 16);
        c.conditionValue.type       = FWP_BYTE_ARRAY16_TYPE;
        c.conditionValue.byteArray16 = &b16;
    } else {
        static FWP_V6_ADDR_AND_MASK am6;
        memcpy(am6.addr, e->addr, 16);
        am6.prefixLength = (UINT8)e->prefix;
        c.conditionValue.type       = FWP_V6_ADDR_MASK;
        c.conditionValue.v6AddrMask  = &am6;
    }
    return add_permit(engine, layer, &c, 1, added);
}

/* Permit a UDP/TCP remote (or local) port on a layer. proto = IPPROTO_UDP/TCP. */
static DWORD add_permit_port(HANDLE engine, const GUID *layer, UINT8 proto,
                             UINT16 port, int local_port, int *added) {
    FWPM_FILTER_CONDITION0 conds[2];
    memset(conds, 0, sizeof(conds));
    conds[0].fieldKey              = FWPM_CONDITION_IP_PROTOCOL;
    conds[0].matchType             = FWP_MATCH_EQUAL;
    conds[0].conditionValue.type   = FWP_UINT8;
    conds[0].conditionValue.uint8  = proto;
    conds[1].fieldKey              = local_port ? FWPM_CONDITION_IP_LOCAL_PORT
                                                : FWPM_CONDITION_IP_REMOTE_PORT;
    conds[1].matchType             = FWP_MATCH_EQUAL;
    conds[1].conditionValue.type   = FWP_UINT16;
    conds[1].conditionValue.uint16 = port;
    return add_permit(engine, layer, conds, 2, added);
}

/* --------------------------------------------------------------------------- */
/* Remove all GuardsArm filters (by provider), then sub-layer + provider.      */
/* Must run inside a transaction.                                              */
/* --------------------------------------------------------------------------- */
static DWORD remove_all_filters(HANDLE engine, int *removed) {
    FWPM_FILTER_ENUM_TEMPLATE0 tmpl;
    memset(&tmpl, 0, sizeof(tmpl));
    tmpl.providerKey        = (GUID *)&GUARDSARM_WFP_PROVIDER;   /* only ours */
    tmpl.numFilterConditions = 0;
    tmpl.actionMask         = 0xFFFFFFFF;

    HANDLE en = NULL;
    DWORD st = FwpmFilterCreateEnumHandle0(engine, &tmpl, &en);
    if (st != ERROR_SUCCESS) {
        /* No provider yet -> nothing to remove. */
        if (st == FWP_E_PROVIDER_NOT_FOUND) return ERROR_SUCCESS;
        return st;
    }

    for (;;) {
        FWPM_FILTER0 **arr = NULL;
        UINT32 count = 0;
        st = FwpmFilterEnum0(engine, en, 64, &arr, &count);
        if (st != ERROR_SUCCESS || count == 0) { if (arr) FwpmFreeMemory0((void **)&arr); break; }
        for (UINT32 i = 0; i < count; i++) {
            if (FwpmFilterDeleteById0(engine, arr[i]->filterId) == ERROR_SUCCESS && removed) (*removed)++;
        }
        FwpmFreeMemory0((void **)&arr);
        if (count < 64) break;
    }
    FwpmFilterDestroyEnumHandle0(engine, en);
    return ERROR_SUCCESS;
}

/* --------------------------------------------------------------------------- */
/* Public API                                                                  */
/* --------------------------------------------------------------------------- */

int wfp_isolate(const wfp_isolation_cfg *cfg, wfp_report *rep) {
    rep_reset(rep);
    if (!cfg || cfg->allow_count == 0) {
        rep_add(rep, "refusing to isolate: empty allow-list (would sever the manager link). ");
        return -1;
    }

    HANDLE engine = NULL;
    DWORD st = engine_open(&engine);
    if (st != ERROR_SUCCESS) { rep_add(rep, "FwpmEngineOpen failed (0x%lX). ", st); return -1; }

    st = FwpmTransactionBegin0(engine, 0);
    if (st != ERROR_SUCCESS) { rep_add(rep, "TransactionBegin failed (0x%lX). ", st); FwpmEngineClose0(engine); return -1; }

    int added = 0;
    DWORD op = ensure_identity(engine);
    if (op != ERROR_SUCCESS) { rep_add(rep, "provider/sublayer add failed (0x%lX). ", op); goto abort; }

    /* Refresh: clear any stale GuardsArm filters first, so re-isolation is clean. */
    remove_all_filters(engine, NULL);

    for (int L = 0; L < 4; L++) {
        const GUID *layer = WFP_ENFORCE_LAYERS[L];
        int is_v4 = (L == WFP_LAYER_CONNECT_V4 || L == WFP_LAYER_ACCEPT_V4);

        /* 1) permit loopback */
        if ((op = add_permit_loopback(engine, layer, &added)) != ERROR_SUCCESS) { rep_add(rep, "loopback permit failed (0x%lX). ", op); goto abort; }

        /* 2) permit each allow-list host/CIDR (matching family) */
        for (size_t a = 0; a < cfg->allow_count; a++) {
            const wfp_allow_entry *e = &cfg->allow[a];
            if (is_v4 && e->family == AF_INET)  { if ((op = add_permit_addr_v4(engine, layer, e, &added)) != ERROR_SUCCESS) { rep_add(rep, "v4 allow add failed (0x%lX). ", op); goto abort; } }
            if (!is_v4 && e->family == AF_INET6){ if ((op = add_permit_addr_v6(engine, layer, e, &added)) != ERROR_SUCCESS) { rep_add(rep, "v6 allow add failed (0x%lX). ", op); goto abort; } }
        }

        /* 3) permit explicit manager ports (both directions cover mgr-initiated) */
        for (size_t p = 0; p < cfg->port_count; p++) {
            add_permit_port(engine, layer, IPPROTO_TCP, (UINT16)cfg->ports[p], 0, &added);
        }

        /* 4) optional DNS (remote 53, udp+tcp) — only on outbound layers */
        if (cfg->allow_dns && (L == WFP_LAYER_CONNECT_V4 || L == WFP_LAYER_CONNECT_V6)) {
            add_permit_port(engine, layer, IPPROTO_UDP, 53, 0, &added);
            add_permit_port(engine, layer, IPPROTO_TCP, 53, 0, &added);
        }

        /* 5) optional DHCP (client local port 68 / server 67) to keep the lease */
        if (cfg->allow_dhcp) {
            add_permit_port(engine, layer, IPPROTO_UDP, 68, 1, &added);  /* local 68 */
            add_permit_port(engine, layer, IPPROTO_UDP, 67, 0, &added);  /* remote 67 */
        }

        /* 6) block everything else on this layer (lowest weight) */
        if ((op = add_block_all(engine, layer, &added)) != ERROR_SUCCESS) { rep_add(rep, "block-all add failed (0x%lX). ", op); goto abort; }
    }

    st = FwpmTransactionCommit0(engine);
    if (st != ERROR_SUCCESS) { rep_add(rep, "TransactionCommit failed (0x%lX). ", st); goto abort; }

    FwpmEngineClose0(engine);
    if (rep) { rep->ok = true; rep->filters_added = added; }
    rep_add(rep, "isolated: %d filters across 4 ALE layers, %zu allow entries. ", added, cfg->allow_count);
    return 0;

abort:
    FwpmTransactionAbort0(engine);
    FwpmEngineClose0(engine);
    return -1;
}

int wfp_unisolate(wfp_report *rep) {
    rep_reset(rep);
    HANDLE engine = NULL;
    DWORD st = engine_open(&engine);
    if (st != ERROR_SUCCESS) { rep_add(rep, "FwpmEngineOpen failed (0x%lX). ", st); return -1; }

    st = FwpmTransactionBegin0(engine, 0);
    if (st != ERROR_SUCCESS) { rep_add(rep, "TransactionBegin failed (0x%lX). ", st); FwpmEngineClose0(engine); return -1; }

    int removed = 0;
    DWORD op = remove_all_filters(engine, &removed);
    if (op != ERROR_SUCCESS && op != FWP_E_PROVIDER_NOT_FOUND) { rep_add(rep, "filter removal failed (0x%lX). ", op); FwpmTransactionAbort0(engine); FwpmEngineClose0(engine); return -1; }

    /* Delete sub-layer + provider (ignore not-found: idempotent). */
    op = FwpmSubLayerDeleteByKey0(engine, &GUARDSARM_WFP_SUBLAYER);
    if (op != ERROR_SUCCESS && op != FWP_E_SUBLAYER_NOT_FOUND) rep_add(rep, "sublayer delete warned (0x%lX). ", op);
    op = FwpmProviderDeleteByKey0(engine, &GUARDSARM_WFP_PROVIDER);
    if (op != ERROR_SUCCESS && op != FWP_E_PROVIDER_NOT_FOUND) rep_add(rep, "provider delete warned (0x%lX). ", op);

    st = FwpmTransactionCommit0(engine);
    if (st != ERROR_SUCCESS) { rep_add(rep, "TransactionCommit failed (0x%lX). ", st); FwpmTransactionAbort0(engine); FwpmEngineClose0(engine); return -1; }

    FwpmEngineClose0(engine);
    if (rep) { rep->ok = true; rep->filters_removed = removed; }
    rep_add(rep, "un-isolated: removed %d GuardsArm filters + sub-layer + provider. ", removed);
    return 0;
}

bool wfp_is_isolated(void) {
    HANDLE engine = NULL;
    if (engine_open(&engine) != ERROR_SUCCESS) return false;
    FWPM_SUBLAYER0 *sl = NULL;
    DWORD st = FwpmSubLayerGetByKey0(engine, &GUARDSARM_WFP_SUBLAYER, &sl);
    bool present = (st == ERROR_SUCCESS && sl != NULL);
    if (sl) FwpmFreeMemory0((void **)&sl);
    FwpmEngineClose0(engine);
    return present;
}

/* Best-effort DNS cache flush via dnsapi (no hard dependency). */
static void flush_dns(void) {
    HMODULE dns = LoadLibraryA("dnsapi.dll");
    if (!dns) return;
    typedef BOOL (WINAPI *flush_t)(void);
    flush_t f = (flush_t)GetProcAddress(dns, "DnsFlushResolverCache");
    if (f) f();
    FreeLibrary(dns);
}

/* Best-effort TCP connectivity probe: connect to 1.1.1.1:443 with a short timeout.
 * Returns 1 if the connection succeeds (network reachable), 0 otherwise. */
static int probe_connectivity(void) {
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return 0;
    int ok = 0;
    SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s != INVALID_SOCKET) {
        u_long nb = 1; ioctlsocket(s, FIONBIO, &nb);
        struct sockaddr_in sa; memset(&sa, 0, sizeof(sa));
        sa.sin_family = AF_INET; sa.sin_port = htons(443);
        sa.sin_addr.s_addr = htonl(0x01010101);   /* 1.1.1.1 */
        connect(s, (struct sockaddr *)&sa, sizeof(sa));
        fd_set wf; FD_ZERO(&wf); FD_SET(s, &wf);
        struct timeval tv; tv.tv_sec = 3; tv.tv_usec = 0;
        if (select(0, NULL, &wf, NULL, &tv) == 1) {
            int err = 0; int len = sizeof(err);
            getsockopt(s, SOL_SOCKET, SO_ERROR, (char *)&err, &len);
            ok = (err == 0);
        }
        closesocket(s);
    }
    WSACleanup();
    return ok;
}

int wfp_repair_network(bool reset_winsock, wfp_report *rep) {
    /* 1. Remove every GuardsArm filter/sub-layer/provider. */
    int r = wfp_unisolate(rep);

    /* 2. Flush DNS. */
    flush_dns();
    rep_add(rep, "DNS cache flushed. ");

    /* 3. Winsock reset is a defensive extra — normal isolation never touches the
     * Winsock catalog, so this is only queued when explicitly requested and needs
     * a reboot to take effect. We do NOT run it inline (it would disrupt sockets).*/
    if (reset_winsock) {
        rep_add(rep, "NOTE: Winsock reset requested — run `netsh winsock reset` + reboot only if sockets are still wedged (not normally needed). ");
    }

    /* 4. Verify connectivity. */
    int net = probe_connectivity();
    rep_add(rep, "connectivity probe (1.1.1.1:443): %s. ", net ? "REACHABLE" : "no route (may be offline)");
    if (rep) rep->ok = (r == 0);
    return r;
}

/* --------------------------------------------------------------------------- */
/* Allow-list text parsing ("addr" or "addr/prefix", v4 or v6)                 */
/* --------------------------------------------------------------------------- */
int wfp_parse_allow(const char *text, wfp_allow_entry *out) {
    if (!text || !out) return -1;
    char buf[128];
    strncpy(buf, text, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    int prefix = -1;
    char *slash = strchr(buf, '/');
    if (slash) { *slash = '\0'; prefix = atoi(slash + 1); }

    memset(out, 0, sizeof(*out));
    struct in6_addr a6;
    struct in_addr  a4;
    if (inet_pton(AF_INET, buf, &a4) == 1) {
        out->family = AF_INET;
        memcpy(out->addr, &a4, 4);
        out->prefix = (prefix < 0 || prefix > 32) ? 32 : prefix;
        return 0;
    }
    if (inet_pton(AF_INET6, buf, &a6) == 1) {
        out->family = AF_INET6;
        memcpy(out->addr, &a6, 16);
        out->prefix = (prefix < 0 || prefix > 128) ? 128 : prefix;
        return 0;
    }
    return -1;
}

#endif /* WIN32 */
