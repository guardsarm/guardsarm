/* Copyright (C) 2026 GuardSarm, Inc.
 * All right reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

/* Active Response shared headers */

#ifndef AR_H
#define AR_H

/* Recipient agents */
#define ALL_AGENTS      0000001
#define REMOTE_AGENT    0000002
#define SPECIFIC_AGENT  0000004
#define AS_ONLY         0000010
#define SPECIFIC_AGENT_SIZED 0000040

/* We now also support non Active Response messages in here */
#define NO_AR_MSG       0000020

#define ALL_AGENTS_C            'A'
#define REMOTE_AGENT_C          'R'
#define SPECIFIC_AGENT_C        'S'
#define SPECIFIC_AGENT_SIZED_C  's'
#define NONE_C                  'N'
#define NO_AR_C                 '!'

/* AR Queues to use */
#define REMOTE_AR       00001
#define LOCAL_AR        00002

#endif /* AR_H */
