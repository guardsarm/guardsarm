/*
 * GuardSarm Module for Agent control
 * Copyright (C) 2026 GuardSarm, Inc.
 * January, 2019
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#if defined(__linux__) || defined(__MACH__) || defined(FreeBSD) || defined(OpenBSD)

#include "wm_control.h"
#include <cJSON.h>
#include "file_op.h"
#include "os_net.h"

static void *gm_control_main();
static void gm_control_destroy();
static void gm_control_stop();
cJSON *gm_control_dump();
static void *process_control();

const gm_context GM_CONTROL_CONTEXT = {
    .name = "control",
    .start = (gm_routine)gm_control_main,
    .destroy = (void(*)(void *))gm_control_destroy,
    .dump = (cJSON * (*)(const void *))gm_control_dump,
    .sync = NULL,
    .stop = (void(*)(void *))gm_control_stop,
    .query = NULL,
};

static void *gm_control_main() {
    mtinfo(GM_CONTROL_LOGTAG, "Starting control thread.");
#ifdef CLIENT
    w_create_thread(process_control, NULL);
#else
    process_control();
#endif
    return NULL;
}

static void gm_control_destroy() {
}

static void gm_control_stop() {
    gm_shutdown_requested = 1;
}

gmodule *gm_control_read() {
    gmodule *module;

    os_calloc(1, sizeof(gmodule), module);
    module->context = &GM_CONTROL_CONTEXT;
    module->tag = strdup(module->context->name);

    return module;
}

cJSON *gm_control_dump() {
    cJSON *root = cJSON_CreateObject();
    cJSON *gm_wd = cJSON_CreateObject();
    cJSON_AddStringToObject(gm_wd, "enabled", "yes");
    cJSON_AddItemToObject(root, "guardsarm_control", gm_wd);
    return root;
}

bool gm_control_check_systemd() {
    if (access("/run/systemd/system", F_OK) != 0) {
        return false;
    }

    FILE *fp = fopen("/proc/1/comm", "r");
    if (fp) {
        char init_name[256];
        if (fgets(init_name, sizeof(init_name), fp)) {
            init_name[strcspn(init_name, "\n")] = 0;
            if (strcmp(init_name, "systemd") == 0) {
                fclose(fp);
                return true;
            }
        }
        fclose(fp);
    }

    return false;
}

bool gm_control_wait_for_service_active(const char *service) {
    const int timeout = 60;
    int elapsed = 0;
    char cmd[256];

    snprintf(cmd, sizeof(cmd), "systemctl is-active %s 2>/dev/null", service);

    while (elapsed < timeout) {
        FILE *fp = popen(cmd, "r");
        if (fp) {
            char state[256];
            if (fgets(state, sizeof(state), fp)) {
                state[strcspn(state, "\n")] = 0;

                if (strcmp(state, "inactive") == 0 || strcmp(state, "failed") == 0) {
                    pclose(fp);
                    mtwarn(GM_CONTROL_LOGTAG, "Service %s is in state '%s', systemctl cannot reload", service, state);
                    return false;
                }

                if (strcmp(state, "active") == 0) {
                    pclose(fp);
                    return true;
                }
            }
            pclose(fp);
        }

        sleep(1);
        elapsed++;
    }

    mterror(GM_CONTROL_LOGTAG, "Service %s is not active after waiting %d seconds", service, timeout);
    return false;
}

const char *gm_control_get_bin(void) {
    /* 5.0 rename: manager ships guardsarm-manager-control, agent ships guardsarm-control. */
#ifdef CLIENT
    return "bin/guardsarm-control";
#else
    return "bin/guardsarm-manager-control";
#endif
}

size_t gm_control_execute_action(const char *action, const char *service, char **output) {
#ifdef __APPLE__
    /* On macOS, do not run guardsarm-control directly from here. Forking it from
     * guardsarm-modulesd makes modulesd the TCC "responsible process" of the
     * respawned daemons, so guardsarm-syscheckd's Full Disk Access entry gets filed
     * under guardsarm-modulesd and syscheckd never appears in the FDA list. Instead
     * we drop a request flag that GuardSarm-launcher (the launchd job anchor, a
     * shell) polls; it runs `guardsarm-control <action>` in the same launchd-clean
     * lineage as boot, so tccd files the FDA entry under guardsarm-syscheckd itself.
     * See src/init/darwin-init.sh (GuardSarm-launcher loop). */
    (void)service;
    {
        /* Path is relative to the install dir, matching the "bin/guardsarm-control"
         * convention used in the non-Apple branch below. We write a temp file
         * and rename() it onto the final flag so the launcher (the reader)
         * never observes a partially written request: rename(2) is atomic
         * within the same directory. */
        const char *flag = "var/run/guardsarm-control.request";
        const char *flag_tmp = "var/run/guardsarm-control.request.tmp";

        FILE *fp = fopen(flag_tmp, "w");
        if (fp == NULL) {
            mterror(GM_CONTROL_LOGTAG, "Cannot create control request flag '%s': %s (%d)", flag_tmp, strerror(errno), errno);
            os_strdup("err Cannot write control flag", *output);
            return strlen(*output);
        }
        if (fprintf(fp, "%s\n", action) < 0) {
            mterror(GM_CONTROL_LOGTAG, "Cannot write control request flag '%s': %s (%d)", flag_tmp, strerror(errno), errno);
            fclose(fp);
            unlink(flag_tmp);
            os_strdup("err Cannot write control flag", *output);
            return strlen(*output);
        }

        if (fclose(fp) != 0) {
            mterror(GM_CONTROL_LOGTAG, "Cannot flush control request flag '%s': %s (%d)", flag_tmp, strerror(errno), errno);
            unlink(flag_tmp);
            os_strdup("err Cannot write control flag", *output);
            return strlen(*output);
        }

        if (rename(flag_tmp, flag) != 0) {
            mterror(GM_CONTROL_LOGTAG, "Cannot commit control request flag '%s': %s (%d)", flag, strerror(errno), errno);
            unlink(flag_tmp);
            os_strdup("err Cannot write control flag", *output);
            return strlen(*output);
        }

        mtinfo(GM_CONTROL_LOGTAG, "Requested '%s' via GuardSarm-launcher (flag '%s')", action, flag);
        os_strdup("ok ", *output);
        return strlen(*output);
    }
#else
    bool use_systemd = gm_control_check_systemd();
    const char *control_bin = gm_control_get_bin();
    char *exec_cmd[4] = {NULL};

    if (use_systemd) {
        exec_cmd[0] = "systemctl";
        exec_cmd[1] = (char *)action;
        exec_cmd[2] = (char *)service;
        mtinfo(GM_CONTROL_LOGTAG, "Executing '%s' on %s using systemctl", action, service);
    } else {
        exec_cmd[0] = (char *)control_bin;
        exec_cmd[1] = (char *)action;
        mtinfo(GM_CONTROL_LOGTAG, "Executing '%s' on %s using %s", action, service, control_bin);
    }

    switch (fork()) {
        case -1:
            mterror(GM_CONTROL_LOGTAG, "Cannot fork for %s", action);
            os_strdup("err Cannot fork", *output);
            return strlen(*output);
        case 0:
            if (use_systemd && strcmp(action, "reload") == 0) {
                if (!gm_control_wait_for_service_active(service)) {
                    mtwarn(GM_CONTROL_LOGTAG, "Service %s not active for systemctl, falling back to %s", service, control_bin);
                    char *fallback_cmd[] = {(char *)control_bin, (char *)action, NULL};
                    if (execvp(fallback_cmd[0], fallback_cmd) < 0) {
                        mterror(GM_CONTROL_LOGTAG, "Error executing %s command via %s: %s (%d)", action, control_bin, strerror(errno), errno);
                    }
                    _exit(1);
                }
            }
            if (execvp(exec_cmd[0], exec_cmd) < 0) {
                mterror(GM_CONTROL_LOGTAG, "Error executing %s command (%s): %s (%d)", action, exec_cmd[0], strerror(errno), errno);
            }
            _exit(1);
        default:
            os_strdup("ok ", *output);
            return strlen(*output);
    }
#endif
}

size_t gm_control_dispatch(char *command, char **output) {
    char *args = strchr(command, ' ');
    if (args) {
        *args = '\0';
        args++;
    }

    mtdebug2(GM_CONTROL_LOGTAG, "Dispatching command: '%s'", command);

#ifdef CLIENT
    const char *service = "guardsarm-agent";
#else
    const char *service = "guardsarm-manager";
#endif

    if (strcmp(command, "restart") == 0) {
        return gm_control_execute_action("restart", service, output);

    } else if (strcmp(command, "reload") == 0) {
        return gm_control_execute_action("reload", service, output);

    } else {
        mterror(GM_CONTROL_LOGTAG, "Unknown command: '%s'", command);
        os_strdup("Err", *output);
        return strlen(*output);
    }
}

static void *process_control() {
    int sock;
    int peer;
    char *buffer = NULL;
    char *response = NULL;
    ssize_t length;
    fd_set fdset;

    if (sock = OS_BindUnixDomainWithPerms(CONTROL_SOCK, SOCK_STREAM, OS_MAXSTR, getuid(), gm_getGroupID(), 0660), sock < 0) {
        mterror(GM_CONTROL_LOGTAG, "Unable to bind to socket '%s': (%d) %s.", CONTROL_SOCK, errno, strerror(errno));
        return NULL;
    }

    while (!gm_shutdown_requested) {

        switch (gm_select_interruptible(sock, &fdset)) {
        case -1:
            mterror_exit(GM_CONTROL_LOGTAG, "At process_control(): select(): %s", strerror(errno));
            break;
        case 0:
            continue;
        default:
            break;
        }

        if (peer = accept(sock, NULL, NULL), peer < 0) {
            if (errno != EINTR) {
                mterror(GM_CONTROL_LOGTAG, "At process_control(): accept(): %s", strerror(errno));
            }
            continue;
        }

        os_calloc(OS_MAXSTR + 1, sizeof(char), buffer);

#ifdef CLIENT
        switch (length = OS_RecvSecureTCP(peer, buffer, OS_MAXSTR), length) {
        case -1:
            mterror(GM_CONTROL_LOGTAG, "At process_control(): OS_RecvSecureTCP(): %s", strerror(errno));
            break;
        case 0:
            mtinfo(GM_CONTROL_LOGTAG, "Empty message from local client.");
            close(peer);
            break;
        case OS_SOCKTERR:
            mterror(GM_CONTROL_LOGTAG, "Received message > %i", MAX_DYN_STR);
            close(peer);
            break;
        default:
            buffer[length] = '\0';
            gm_control_dispatch(buffer, &response);
            if (response) {
                OS_SendSecureTCP(peer, strlen(response), response);
                free(response);
            } else {
                OS_SendSecureTCP(peer, 3, "Err");
            }
            close(peer);
        }
#else
        switch (length = OS_RecvUnix(peer, OS_MAXSTR, buffer), length) {
        case -1:
            mterror(GM_CONTROL_LOGTAG, "At process_control(): OS_RecvUnix(): %s", strerror(errno));
            break;
        case 0:
            mtinfo(GM_CONTROL_LOGTAG, "Empty message from local client.");
            close(peer);
            break;
        case OS_MAXLEN:
            mterror(GM_CONTROL_LOGTAG, "Received message > %i", MAX_DYN_STR);
            close(peer);
            break;
        default:
            gm_control_dispatch(buffer, &response);
            if (response) {
                OS_SendUnix(peer, response, 0);
                free(response);
            } else {
                OS_SendUnix(peer, "Err", 4);
            }
            close(peer);
        }
#endif

        free(buffer);
        buffer = NULL;
    }

    /* Intentional cleanup code: the while(1) loop above currently exits only via
     * merror_exit(). This block ensures proper resource release if a graceful
     * exit path is added in the future. */
    // coverity[unreachable]
    close(sock);
    return NULL;
}

#endif
