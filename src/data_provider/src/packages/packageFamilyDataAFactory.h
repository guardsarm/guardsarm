/*
 * GuardSarm SYSINFO
 * Copyright (C) 2026 GuardSarm, Inc.
 * December 14, 2020.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef _PACKAGE_FAMILY_DATA_AFACTORY_H
#define _PACKAGE_FAMILY_DATA_AFACTORY_H

#include <memory>
#include "json.hpp"
#include "packageMac.h"
#include "sharedDefs.h"

template <OSPlatformType osType>
class FactoryPackageFamilyCreator final
{
    public:
        static std::shared_ptr<IPackage> create(const std::pair<PackageContext, int>& /*ctx*/)
        {
            throw std::runtime_error
            {
                "Error creating package data retriever."
            };
        }

        static std::shared_ptr<IPackage> create(const std::shared_ptr<IPackageWrapper>& /*pkgwrapper*/)
        {
            throw std::runtime_error
            {
                "Error creating package data retriever."
            };
        }
};

template <>
class FactoryPackageFamilyCreator<OSPlatformType::BSDBASED> final
{
    public:
        static std::shared_ptr<IPackage> create(const std::pair<PackageContext, int>& ctx)
        {
            return FactoryBSDPackage::create(ctx);
        }
        static std::shared_ptr<IPackage> create(const std::pair<SQLite::IStatement&, const int>& ctx)
        {
            return FactoryBSDPackage::create(ctx);
        }
};

#endif // _PACKAGE_FAMILY_DATA_AFACTORY_H
