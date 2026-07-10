/*
 * GuardSarm SYSINFO
 * Copyright (C) 2026 GuardSarm, Inc.
 * July 29, 2023.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef _MODERN_PACKAGE_DATA_RETRIEVER_HPP
#define _MODERN_PACKAGE_DATA_RETRIEVER_HPP

#include "json.hpp"
#include "sharedDefs.h"
#include <functional>
#include <map>
#include <unordered_set>

#include "packages/packagesNPM.hpp"
#include "packages/packagesPYPI.hpp"

// Extract package information in fully compatible Linux systems
class ModernFactoryPackagesCreator final
{
    public:
        static void getPackages(const std::map<std::string, std::set<std::string>>& paths, std::function<void(nlohmann::json&)> callback, const std::unordered_set<std::string>& excludePaths = {})
        {
            auto cbCopy {callback};
            PYPI().getPackages(paths.at("PYPI"), std::move(callback), excludePaths);
            NPM().getPackages(paths.at("NPM"), std::move(cbCopy));
        }
};

#endif  // _MODERN_PACKAGE_DATA_RETRIEVER_HPP
