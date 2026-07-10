/* Copyright (C) 2026 GuardSarm, Inc.
 * Copyright (C) 2009 Trend Micro Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef OS_WIN_H
#define OS_WIN_H

/* Install the GUARDSARM-HIDS agent service */
int InstallService(char *path);

/* Uninstall the GUARDSARM-HIDS agent service */
int UninstallService();

/* Check if the GUARDSARM-HIDS agent service is running
 * Returns 1 on success (running) or 0 if not running
 */
int CheckServiceRunning();

/* Start GUARDSARM-HIDS service */
int os_start_service();

/* Stop GUARDSARM-HIDS service */
int os_stop_service();

/* Start the process from the services */
int os_WinMain(int argc, char **argv);

/* Locally start the process (after the services initialization) */
int local_start();

#endif /* OS_WIN_H */
