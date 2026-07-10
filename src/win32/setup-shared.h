/* Copyright (C) 2026 GuardSarm, Inc.
 * Copyright (C) 2009 Trend Micro Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#include "file_op.h"
#include "os_regex.h"
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <windows.h>
#include <winsock2.h>

#define GUARDSARMCONF  "ossec.conf"
#define OSSECDEF   "default-ossec.conf"
#define OSSECLAST  "ossec.conf.bak"
#define CLIENTKEYS "client.keys"
#define OS_MAXSTR  1024

/* Check if a file exists */
int fileexist(char* file);

/* Grep for a string in a file */
int dogrep(char* file, char* str);

/* Check if dir exists */
int direxist(char* dir);

/* Get Windows main directory */
void get_win_dir(char* file, int f_size);
