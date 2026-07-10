/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef WIN32

#ifndef NETBUFFER_WRAPPERS_H
#define NETBUFFER_WRAPPERS_H

#include "../../../../remoted/include/remoted.h"

void __wrap_nb_close(__attribute__((unused)) netbuffer_t * buffer, int sock);

void __wrap_nb_open(__attribute__((unused)) netbuffer_t * buffer, int sock, const struct sockaddr_storage * peer_info);

int __wrap_nb_queue(__attribute__((unused)) netbuffer_t * buffer, int socket, char * crypt_msg, ssize_t msg_size, char * agent_id);

int __wrap_nb_recv(__attribute__((unused)) netbuffer_t * buffer, int sock);

int __wrap_nb_send(__attribute__((unused)) netbuffer_t * buffer, int sock);

#endif

#endif
