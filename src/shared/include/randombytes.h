/*
 * Copyright (C) 2026 GuardSarm, Inc.
 * Contributed by Jeremy Rossi (@jrossi)
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef RANDOMBYTES_H
#define RANDOMBYTES_H

void randombytes(void *ptr, size_t length);
void srandom_init(void);
int os_random(void);

#endif /* RANDOMBYTES_H */
