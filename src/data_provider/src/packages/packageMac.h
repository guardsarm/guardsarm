/*
 * GuardSarm SYSINFO
 * Copyright (C) 2026 GuardSarm, Inc.
 * December 14, 2020.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef _PACKAGE_MAC_H
#define _PACKAGE_MAC_H

#include "ipackageInterface.h"
#include "ipackageWrapper.h"
#include "sqliteWrapperTemp.h"

struct PackageContext
{
    std::string filePath;
    std::string package;
    std::string version;
};

class FactoryBSDPackage
{
    public:
        static std::shared_ptr<IPackage>create(const std::pair<PackageContext, int>& ctx);
        static std::shared_ptr<IPackage>create(const std::pair<SQLite::IStatement&, const int>& ctx);
};

class BSDPackageImpl final : public IPackage
{
        const std::shared_ptr<IPackageWrapper> m_packageWrapper;
    public:
        explicit BSDPackageImpl(const std::shared_ptr<IPackageWrapper>& packageWrapper);

        void buildPackageData(nlohmann::json& package) override;
};

#endif // _PACKAGE_MAC_H
