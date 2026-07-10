/* Copyright (C) 2026 GuardSarm, Inc.
 * Copyright (C) 2009 Trend Micro Inc.
 * All right reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

/* Functions to handle signal manipulation */

#ifndef SIG_H
#define SIG_H

void HandleSIG(int sig) __attribute__((noreturn));
void HandleSIGPIPE(int sig);
void HandleExit();

/* Start signal manipulation */
void StartSIG(const char *process_name) __attribute__((nonnull));

/* Start signal manipulation -- function as an argument */
void StartSIG2(const char *process_name, void (*func)(int)) __attribute__((nonnull));

#endif /* SIG_H */
