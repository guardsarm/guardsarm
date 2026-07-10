/*
 * GuardSarm SysCollector
 * Copyright (C) 2026 GuardSarm, Inc.
 * January 12, 2021.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#pragma once

#include <json.hpp>
#include <string>
#include <map>

class SysNormalizer
{
    public:
        SysNormalizer(const std::string& configFile,
                      const std::string& target);
        ~SysNormalizer() = default;
        void normalize(const std::string& type,
                       nlohmann::json& data) const;
        void removeExcluded(const std::string& type,
                            nlohmann::json& data) const;
    private:
        static std::map<std::string, nlohmann::json> getTypeValues(const std::string& configFile,
                                                                   const std::string& target,
                                                                   const std::string& type);
        const std::map<std::string, nlohmann::json> m_typeExclusions;
        const std::map<std::string, nlohmann::json> m_typeDictionary;
};
