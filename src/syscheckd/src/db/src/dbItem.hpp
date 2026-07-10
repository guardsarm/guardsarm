/*
 * GuardSarm Syscheck
 * Copyright (C) 2026 GuardSarm, Inc.
 * September 23, 2021.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef _DBITEM_HPP
#define _DBITEM_HPP
#include "fimDBSpecialization.h"
#include "json.hpp"
#include "syscheck.h"

class DBItem
{
    public:
        DBItem(const std::string& identifier, const std::string& checksum)
            : m_identifier(identifier)
            , m_checksum(checksum)
        {
            FIMDBCreator<OS_TYPE>::encodeString(m_identifier);
            m_oldData = false;
        }

        // LCOV_EXCL_START
        virtual ~DBItem() = default;
        // LCOV_EXCL_STOP
        virtual fim_entry* toFimEntry() = 0;
        virtual const nlohmann::json* toJSON() const = 0;

    protected:
        std::string m_identifier;
        std::string m_checksum;
        bool m_oldData;
};
#endif //_DBITEM_HPP
