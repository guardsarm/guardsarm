/*
 * GuardSarm Module for SQLite database syncing
 * Copyright (C) 2026 GuardSarm, Inc.
 * November 29, 2016
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef GM_DATABASE
#define GM_DATABASE

#define GM_DATABASE_LOGTAG ARGV0 ":database"

typedef struct gm_database {
    int sync_agents;
    int real_time;
    int interval;
    int max_queued_events;
} gm_database;

extern int wdb_wmdb_sock;

// Read configuration and return a module (if enabled) or NULL (if disabled)
gmodule* gm_database_read();

/**
 * @brief Synchronizes a keystore with the agent table of global.db. It will insert
 *        the agents that are in the keystore and are not in global.db.
 *        In addition it will remove from global.db in guardsarm-db all the agents that
 *        are not in the keystore. Also it will remove all the artifacts for those
 *        agents.
 *
 * @param keys The keystore structure to be synchronized
 */
void sync_keys_with_wdb(keystore *keys);

#endif /* WM_DATABASE */
