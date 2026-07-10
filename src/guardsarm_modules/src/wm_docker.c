/*
 * GuardSarm Module for Docker integration
 * Copyright (C) 2026 GuardSarm, Inc.
 * October, 2018.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef WIN32

#include "wmodules.h"

static gm_docker_t *docker_conf;                               // Pointer to docker config struct

static void* gm_docker_main(gm_docker_t *docker_conf);         // Module main function. It won't return
static void gm_docker_setup(gm_docker_t *_docker_conf);        // Setup module
static void gm_docker_cleanup();                               // Cleanup function, doesn't overwrite wm_cleanup
static void gm_docker_check();                                 // Check configuration, disable flag
static void gm_docker_destroy(gm_docker_t *docker_conf);       // Destroy data
cJSON *gm_docker_dump(const gm_docker_t *docker_conf);         // Dump docker config to JSON

// Docker module context definition

const gm_context GM_DOCKER_CONTEXT = {
    .name = "docker-listener",
    .start = (gm_routine)gm_docker_main,
    .destroy = (void(*)(void *))gm_docker_destroy,
    .dump = (cJSON * (*)(const void *))gm_docker_dump,
    .sync = NULL,
    .stop = NULL,
    .query = NULL,
};

// Module module main function. It won't return.

void* gm_docker_main(gm_docker_t *docker_conf) {
    int status = 0;
    char * command = GM_DOCKER_SCRIPT_PATH;
    char * timestamp = NULL;
    int attempts = 0;

    gm_docker_setup(docker_conf);
    mtinfo(GM_DOCKER_LOGTAG, "Module docker-listener started.");

    // Main
    do {
        const time_t time_sleep = sched_scan_get_time_until_next_scan(&(docker_conf->scan_config), GM_DOCKER_LOGTAG, docker_conf->flags.run_on_start);

        if (time_sleep) {
            const int next_scan_time = sched_get_next_scan_time(docker_conf->scan_config);
            timestamp = w_get_timestamp(next_scan_time);
            mtdebug2(GM_DOCKER_LOGTAG, "Sleeping until: %s", timestamp);
            os_free(timestamp);
            gm_sleep_until_interruptible(next_scan_time);
            if (gm_shutdown_requested) break;
        }
        if (gm_shutdown_requested) break;
        mtinfo(GM_DOCKER_LOGTAG, "Starting to listening Docker events.");

        // Running the docker listener script

        mtdebug1(GM_DOCKER_LOGTAG, "Launching command '%s'", command);

        wfd_t * wfd = wpopenl(command, W_BIND_STDERR, command, NULL);

        if (wfd == NULL) {
            mterror(GM_DOCKER_LOGTAG, "Cannot launch Docker integration due to an internal error.");
            pthread_exit(NULL);
        }

#ifdef WIN32
        gm_append_handle(wfd->pinfo.hProcess);
#else
        if (0 <= wfd->pid) {
            gm_append_sid(wfd->pid);
        }
#endif

        char buffer[4096];

        while (fgets(buffer, sizeof(buffer), wfd->file_out)) {
            char * end = strchr(buffer, '\n');
            if (end) {
                *end = '\0';
            }

            if (strncmp(buffer, "INFO ", 5) == 0) {
                mtinfo(GM_DOCKER_LOGTAG, "%s", buffer + 5);
            } else if (strncmp(buffer, "WARN ", 5) == 0) {
                mtwarn(GM_DOCKER_LOGTAG, "%s", buffer + 5);
            } else {
                mterror(GM_DOCKER_LOGTAG, "%s", buffer);
            }
        }

        // At this point, DockerListener terminated
#ifdef WIN32
        gm_remove_handle(wfd->pinfo.hProcess);
#else
        if (0 <= wfd->pid) {
            gm_remove_sid(wfd->pid);
        }
#endif
        status = wpclose(wfd);
        int exitcode = WEXITSTATUS(status);

        switch (exitcode) {
        case 127:
            mterror(GM_DOCKER_LOGTAG, "Cannot launch Docker integration. Please check the file '%s'", command);
            pthread_exit(NULL);

        default:
            if (++attempts >= docker_conf->attempts) {
                mterror(GM_DOCKER_LOGTAG, "Maximum attempts reached to run the listener. Exiting...");
                pthread_exit(NULL);
            }
            mtwarn(GM_DOCKER_LOGTAG, "Docker-listener finished unexpectedly (code %d). Retrying to run in next scheduled time...", exitcode);
        }
    } while (FOREVER() && !gm_shutdown_requested);

    return NULL;
}


// Get read data

cJSON *gm_docker_dump(const gm_docker_t *docker_conf) {

    cJSON *root = cJSON_CreateObject();
    cJSON *gm_docker = cJSON_CreateObject();

    sched_scan_dump(&(docker_conf->scan_config), gm_docker);

    if (docker_conf->flags.enabled) cJSON_AddStringToObject(gm_docker,"disabled","no"); else cJSON_AddStringToObject(gm_docker,"disabled","yes");
    if (docker_conf->flags.run_on_start) cJSON_AddStringToObject(gm_docker,"run_on_start","yes"); else cJSON_AddStringToObject(gm_docker,"run_on_start","no");
    cJSON_AddNumberToObject(gm_docker, "attempts", docker_conf->attempts);
    cJSON_AddItemToObject(root,"docker-listener",gm_docker);

    return root;
}


// Destroy data

void gm_docker_destroy(gm_docker_t *docker_conf) {
    free(docker_conf);
}

// Setup module

void gm_docker_setup(gm_docker_t *_docker_conf) {

    docker_conf = _docker_conf;
    gm_docker_check();

    // Cleanup exiting

    atexit(gm_docker_cleanup);
}


// Check configuration

void gm_docker_check() {
    // Check if disabled

    if (!docker_conf->flags.enabled) {
        mtinfo(GM_DOCKER_LOGTAG, "Module disabled. Exiting...");
        pthread_exit(NULL);
    }

    // Check if interval defined; otherwise set default

    if (!docker_conf->interval)
        docker_conf->interval = GM_DOCKER_DEF_INTERVAL;

}

// Cleanup function, doesn't overwrite wm_cleanup

void gm_docker_cleanup() {
    mtinfo(GM_DOCKER_LOGTAG, "Module finished.");
}

#endif
