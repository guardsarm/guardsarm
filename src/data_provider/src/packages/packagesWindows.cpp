/*
 * GuardSarm SYSINFO
 * Copyright (C) 2026 GuardSarm, Inc.
 * January 24, 2022.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#include "packagesWindows.h"
#include "appxWindowsWrapper.h"
#include "sharedDefs.h"
#include "timeHelper.h"

std::shared_ptr<IPackage> FactoryWindowsPackage::create(const HKEY key, const std::string& userId, const std::string& nameApp, const std::set<std::string>& cacheRegistry)
{
    return std::make_shared<WindowsPackageImpl>(std::make_shared<AppxWindowsWrapper>(key, userId, nameApp, cacheRegistry));
}

WindowsPackageImpl::WindowsPackageImpl(const std::shared_ptr<IPackageWrapper>& packageWrapper)
    : m_packageWrapper(packageWrapper)
{ }

void WindowsPackageImpl::buildPackageData(nlohmann::json& package)
{
    package["name"] = m_packageWrapper->name();
    package["version_"] = m_packageWrapper->version();
    package["vendor"] = m_packageWrapper->vendor();
    auto installed = m_packageWrapper->install_time();
    package["installed"] = installed == UNKNOWN_VALUE ? UNKNOWN_VALUE : Utils::timestampToISO8601(installed);
    package["path"] = m_packageWrapper->location();
    package["architecture"] = m_packageWrapper->architecture();
    package["category"] = m_packageWrapper->groups();
    package["description"] = m_packageWrapper->description();
    package["size"] = m_packageWrapper->size();
    package["priority"] = m_packageWrapper->priority();
    package["source"] = m_packageWrapper->source();
    package["type"] = m_packageWrapper->format();
}
