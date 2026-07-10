/*
 * GuardSarm SYSINFO
 * Copyright (C) 2026 GuardSarm, Inc.
 * January 24, 2022.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef _PACKAGE_WINDOWS_H
#define _PACKAGE_WINDOWS_H

#include <winsock2.h>
#include <windows.h>
#include <set>
#include "ipackageInterface.h"
#include "ipackageWrapper.h"

class FactoryWindowsPackage
{
    public:
        static std::shared_ptr<IPackage>create(const HKEY key, const std::string& userId, const std::string& nameApp, const std::set<std::string>& cacheRegistry);
};

class WindowsPackageImpl final : public IPackage
{
    private:
        const std::shared_ptr<IPackageWrapper> m_packageWrapper;
    public:
        explicit WindowsPackageImpl(const std::shared_ptr<IPackageWrapper>& packageWrapper);

        void buildPackageData(nlohmann::json& package) override;
};

#endif // _PACKAGE_WINDOWS_H

