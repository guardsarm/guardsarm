/*
 * GuardSarm Module Manager
 * Copyright (C) 2026 GuardSarm, Inc.
 * November 11, 2020.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef W_MODULES_DEFS
#define W_MODULES_DEFS

#include <pthread.h>
#include <cJSON.h>

#ifdef WIN32
#include <winsock2.h>
#include <windows.h>
#endif

#ifndef ARGV0
#define ARGV0 "guardsarm-modulesd"
#endif // ARGV0

#ifdef WIN32
typedef DWORD WINAPI (*gm_routine)(void*);  // Standard routine pointer
#else
typedef void* (*gm_routine)(void*);         // Standard routine pointer
#endif

// Module context: this should be defined for every module

typedef struct gm_context {
    const char *name;                           // Name for module
    gm_routine start;                           // Main function
    void (*destroy)(void *);                    // Configuration destructor
    cJSON *(* dump)(const void *);              // Dump current configuration
    int (* sync)(const char*, size_t);          // Sync
    void (*stop)(void *);                       // Module destructor
    size_t (*query)(void *, char *, char **);   // Run a query
} gm_context;

// Main module structure

typedef struct gmodule {
    pthread_t thread;                   // Thread ID
    const gm_context *context;          // Context (common structure)
    char *tag;                          // Module tag
    void *data;                         // Data (module-dependent structure)
    struct gmodule *next;               // Pointer to next module
} gmodule;

#endif //W_MODULES_DEFS
