/*
 * GuardSarm DB Query Builder
 * Copyright (C) 2026 GuardSarm, Inc.
 * October 31, 2023.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef _GUARDSARM_DB_QUERY_BUILDER_HPP
#define _GUARDSARM_DB_QUERY_BUILDER_HPP

#include "builder.hpp"
#include "stringHelper.h"
#include <string>

constexpr auto GUARDSARM_DB_ALLOWED_CHARS {"-_ "};

class GuardSarmDBQueryBuilder final : public Utils::Builder<GuardSarmDBQueryBuilder>
{
private:
    std::string m_query;

public:
    GuardSarmDBQueryBuilder& global()
    {
        m_query += "global sql ";
        return *this;
    }

    GuardSarmDBQueryBuilder& agent(const std::string& id)
    {
        if (!Utils::isNumber(id))
        {
            throw std::runtime_error("Invalid agent id");
        }

        m_query += "agent " + id + " sql ";
        return *this;
    }

    GuardSarmDBQueryBuilder& selectAll()
    {
        m_query += "SELECT * ";
        return *this;
    }

    GuardSarmDBQueryBuilder& fromTable(const std::string& table)
    {
        if (!Utils::isAlphaNumericWithSpecialCharacters(table, GUARDSARM_DB_ALLOWED_CHARS))
        {
            throw std::runtime_error("Invalid table name");
        }
        m_query += "FROM " + table + " ";
        return *this;
    }

    GuardSarmDBQueryBuilder& whereColumn(const std::string& column)
    {
        if (!Utils::isAlphaNumericWithSpecialCharacters(column, GUARDSARM_DB_ALLOWED_CHARS))
        {
            throw std::runtime_error("Invalid column name");
        }
        m_query += "WHERE " + column + " ";
        return *this;
    }

    GuardSarmDBQueryBuilder& isNull()
    {
        m_query += "IS NULL ";
        return *this;
    }

    GuardSarmDBQueryBuilder& isNotNull()
    {
        m_query += "IS NOT NULL ";
        return *this;
    }

    GuardSarmDBQueryBuilder& equalsTo(const std::string& value)
    {
        if (!Utils::isAlphaNumericWithSpecialCharacters(value, GUARDSARM_DB_ALLOWED_CHARS))
        {
            throw std::runtime_error("Invalid value");
        }
        m_query += "= '" + value + "' ";
        return *this;
    }

    GuardSarmDBQueryBuilder& andColumn(const std::string& column)
    {
        if (!Utils::isAlphaNumericWithSpecialCharacters(column, GUARDSARM_DB_ALLOWED_CHARS))
        {
            throw std::runtime_error("Invalid column name");
        }
        m_query += "AND " + column + " ";
        return *this;
    }

    GuardSarmDBQueryBuilder& orColumn(const std::string& column)
    {
        if (!Utils::isAlphaNumericWithSpecialCharacters(column, GUARDSARM_DB_ALLOWED_CHARS))
        {
            throw std::runtime_error("Invalid column name");
        }
        m_query += "OR " + column + " ";
        return *this;
    }

    GuardSarmDBQueryBuilder& globalGetCommand(const std::string& command)
    {
        if (!Utils::isAlphaNumericWithSpecialCharacters(command, GUARDSARM_DB_ALLOWED_CHARS))
        {
            throw std::runtime_error("Invalid command");
        }
        m_query += "global get-" + command + " ";
        return *this;
    }

    GuardSarmDBQueryBuilder& globalFindCommand(const std::string& command)
    {
        if (!Utils::isAlphaNumericWithSpecialCharacters(command, GUARDSARM_DB_ALLOWED_CHARS))
        {
            throw std::runtime_error("Invalid command");
        }
        m_query += "global find-" + command + " ";
        return *this;
    }

    GuardSarmDBQueryBuilder& globalSelectCommand(const std::string& command)
    {
        if (!Utils::isAlphaNumericWithSpecialCharacters(command, GUARDSARM_DB_ALLOWED_CHARS))
        {
            throw std::runtime_error("Invalid command");
        }
        m_query += "global select-" + command + " ";
        return *this;
    }

    GuardSarmDBQueryBuilder& agentGetOsInfoCommand(const std::string& id)
    {
        if (!Utils::isNumber(id))
        {
            throw std::runtime_error("Invalid agent id");
        }
        m_query += "agent " + id + " osinfo get ";
        return *this;
    }

    GuardSarmDBQueryBuilder& agentGetHotfixesCommand(const std::string& id)
    {
        if (!Utils::isNumber(id))
        {
            throw std::runtime_error("Invalid agent id");
        }
        m_query += "agent " + id + " hotfix get ";
        return *this;
    }

    GuardSarmDBQueryBuilder& agentGetPackagesCommand(const std::string& id)
    {
        if (!Utils::isNumber(id))
        {
            throw std::runtime_error("Invalid agent id");
        }
        m_query += "agent " + id + " package get ";
        return *this;
    }

    std::string build()
    {
        return m_query;
    }
};

#endif /* _GUARDSARM_DB_QUERY_BUILDER_HPP */
