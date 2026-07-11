/* Copyright (C) 2026 GuardSarm, Inc.
 * All right reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

/* Active response: isolate — network containment. Drops all new connections while
 * preserving loopback + already-ESTABLISHED sessions (so the agent<->manager link
 * survives and the host stays manageable). ENABLE isolates; DISABLE restores.
 * Linux: dedicated iptables chains (cleanly reversible, other rules untouched).
 * Windows: Defender Firewall block-all policy. */

#include "active_responses.h"

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
    // Windows: block all inbound+outbound via Defender Firewall (restore = allow).
    // NOTE: resolve "netsh.exe" (with extension, as block-ip-windows.c does) — a bare
    // "netsh" may not resolve for CreateProcess, and check the child exit code so a failed
    // netsh is reported instead of silently claiming isolation. Absolute System32 fallback.
    char *cmd_path = NULL;
    if (get_binary_path("netsh.exe", &cmd_path) < 0) {
        os_free(cmd_path);
        cmd_path = NULL;
        char sysdir[OS_MAXSTR] = {0};
        if (GetSystemDirectory(sysdir, OS_MAXSTR)) {
            char full[OS_MAXSTR];
            snprintf(full, OS_MAXSTR - 1, "%s\\netsh.exe", sysdir);
            os_strdup(full, cmd_path);
        }
    }
    if (!cmd_path) {
        write_debug_file(argv[0], "netsh not found");
        cJSON_Delete(input_json); return OS_INVALID;
    }
    {
        char dbg[OS_MAXSTR];
        snprintf(dbg, OS_MAXSTR - 1, "Using netsh at '%s'", cmd_path);
        write_debug_file(argv[0], dbg);
    }
    const char *policy = (action == ENABLE_COMMAND) ? "blockinbound,blockoutbound" : "blockinbound,allowoutbound";
    char *pargs[7] = { cmd_path, "advfirewall", "set", "allprofiles", "firewallpolicy", (char *)policy, NULL };
    int rc = -1;
    wfd_t *wfd = wpopenv(cmd_path, pargs, W_BIND_STDERR);
    if (wfd) {
        rc = wpclose(wfd);
    } else {
        write_debug_file(argv[0], "Unable to execute netsh advfirewall set firewallpolicy");
    }
    if (rc == 0) {
        write_debug_file(argv[0], action == ENABLE_COMMAND ? "Isolated (firewall block policy)" : "Un-isolated (firewall restored)");
    } else {
        char m[OS_MAXSTR];
        snprintf(m, OS_MAXSTR - 1, "netsh firewallpolicy set returned exit code %d", rc);
        write_debug_file(argv[0], m);
    }
    os_free(cmd_path);
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
