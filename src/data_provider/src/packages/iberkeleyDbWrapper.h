/*
 * GuardSarm SYSINFO
 * Copyright (C) 2026 GuardSarm, Inc.
 * March 16, 2021.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef _IBERKELEY_DB_WRAPPER_H
#define _IBERKELEY_DB_WRAPPER_H

#include <memory>
#include <cstring>
#include "db.h"

class IBerkeleyDbWrapper
{
    public:
        virtual int32_t getRow(DBT& key, DBT& data) = 0;
        // LCOV_EXCL_START
        virtual ~IBerkeleyDbWrapper() = default;
        // LCOV_EXCL_STOP
        IBerkeleyDbWrapper() = default;
};

#endif // _IBERKELEY_DB_WRAPPER_H
