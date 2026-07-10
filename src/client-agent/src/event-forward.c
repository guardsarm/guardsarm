/* Copyright (C) 2026 GuardSarm, Inc.
 * Copyright (C) 2009 Trend Micro Inc.
 * All right reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#include "shared.h"
#include "agentd.h"
#include "sendmsg.h"
#include "state.h"
#include "os_net.h"
#include "sec.h"


/* Receive a message locally on the agent and forward it to the manager */
void *EventForward()
{

    ssize_t recv_b;
    char msg[OS_MAXSTR + 1];

    /* Initialize variables */
    msg[0] = '\0';
    msg[OS_MAXSTR] = '\0';

    while ((recv_b = recv(agt->m_queue, msg, OS_MAXSTR, MSG_DONTWAIT)) > 0) {
        if (agt->buffer){
            if (msg[0] == 's') {
                if (buffer_append(msg, recv_b) < 0) {
                    break;
                }
            } else {
                msg[recv_b] = '\0';
                if (buffer_append(msg, -1) < 0) {
                    break;
                }
            }
        }else{
            w_agentd_state_update(INCREMENT_MSG_COUNT, NULL);

            if (msg[0] == 's') {
                if (send_msg(msg, recv_b) < 0) break;
            } else {
                msg[recv_b] = '\0';
                if (send_msg(msg, -1) < 0) break;
            }
        }

    }

    return (NULL);
}
