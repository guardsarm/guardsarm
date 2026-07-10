/*
 * GuardSarm SYSINFO
 * Copyright (C) 2026 GuardSarm, Inc.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#include "UNIXSocketRequest.hpp"
#include "packageLinuxParserHelper.h"
#include "sharedDefs.h"

void getSnapInfo(std::function<void(nlohmann::json&)> callback)
{
    const auto onSuccess = [&](const std::string & result)
    {
        auto feed = nlohmann::json::parse(result, nullptr, false).at("result");

        if (feed.is_discarded())
        {
            std::cerr << "Error parsing JSON feed\n";
        }

        for (const auto& entry : feed)
        {
            nlohmann::json mapping = PackageLinuxHelper::parseSnap(entry);

            if (!mapping.empty())
            {
                callback(mapping);
            }
        }
    };

    const auto onError = [&](const std::string & result, const long responseCode, const std::string & responseBody)
    {
        std::cerr << "Error retrieving packages using snap unix-socket (" << responseCode << ") " << result << "\n";
        std::cerr << "Response body: " << responseBody << "\n";
    };

    const auto url = HttpUnixSocketURL("/run/snapd.socket", "http://localhost/v2/snaps");

    UNIXSocketRequest::instance().get(RequestParameters {.url = url},
                                      PostRequestParameters {.onSuccess = onSuccess, .onError = onError},
                                      ConfigurationParameters {});
}
