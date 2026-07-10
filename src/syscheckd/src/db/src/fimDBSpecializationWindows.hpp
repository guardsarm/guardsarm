/*
 * GuardSarm Syscheck
 * Copyright (C) 2022, Wazuh Inc.
 * Copyright (C) 2026 GuardSarm, Inc.
 * March 6, 2022.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef _FIMDB_OS_WINDOWS_SPECIALIZATION_H
#define _FIMDB_OS_WINDOWS_SPECIALIZATION_H

#include <string>

class WindowsSpecialization final
{
    public:
        static const std::string registryTypeToText(const int type);
        static bool isUTF8String(const std::string& str);
        static void encodeString(std::string& str);
};

#endif // _FIMDB_OS_WINDOWS_SPECIALIZATION_H
