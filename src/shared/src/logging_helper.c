/*
 * GuardSarm shared modules utils
 * Copyright (C) 2026 GuardSarm, Inc.
 * Oct 6, 2021.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#include "logging_helper.h"
#include "debug_op.h"

void loggingErrorFunction(const char * log) {
    if (log) {
        merror("%s", log);
    }
}

void taggedLogFunction(modules_log_level_t level, const char* log, const char* tag) {

    switch(level) {
        case LOG_ERROR:
            mterror(tag, "%s", log);
            break;
        case LOG_ERROR_EXIT:
            mterror_exit(tag, "%s", log);
            break;
        case LOG_INFO:
            mtinfo(tag, "%s", log);
            break;
        case LOG_WARNING:
            mtwarn(tag, "%s", log);
            break;
        case LOG_DEBUG:
            mtdebug1(tag, "%s", log);
            break;
        case LOG_DEBUG_VERBOSE:
            mtdebug2(tag, "%s", log);
            break;
        default:;
    }
}

void loggingFunction(modules_log_level_t level, const char* log) {

    switch(level) {
        case LOG_ERROR:
            merror("%s", log);
            break;
        case LOG_ERROR_EXIT:
            merror_exit("%s", log);
            break;
        case LOG_INFO:
            minfo("%s", log);
            break;
        case LOG_WARNING:
            mwarn("%s", log);
            break;
        case LOG_DEBUG:
            mdebug1("%s", log);
            break;
        case LOG_DEBUG_VERBOSE:
            mdebug2("%s", log);
            break;
        default:;
    }
}
