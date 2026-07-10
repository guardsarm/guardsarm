/*
 * GuardSarm inventory sync
 * Copyright (C) 2026 GuardSarm, Inc.
 * August 6, 2025.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#include "inventorySync.hpp"
#include "cjsonSmartDeleter.hpp"
#include "inventorySyncFacade.hpp"

namespace Log
{
    std::function<void(const int, const char*, const char*, const int, const char*, const char*, va_list)>
        GLOBAL_LOG_FUNCTION;
} // namespace Log

void InventorySync::start(
    const std::function<void(const int, const char*, const char*, const int, const char*, const char*, va_list)>&
        logFunction,
    const nlohmann::json& configuration) const
{

    InventorySyncFacade::instance().start(logFunction, configuration);
}

void InventorySync::stop() const
{
    InventorySyncFacade::instance().stop();
}

#ifdef __cplusplus
extern "C"
{
#endif
    void inventory_sync_start(full_log_fnc_t callbackLog, const cJSON* configuration)
    {
        try
        {
            nlohmann::json configurationNlohmann;
            if (configuration)
            {
                const std::unique_ptr<char, CJsonSmartFree> spJsonBytes {cJSON_Print(configuration)};
                configurationNlohmann = nlohmann::json::parse(spJsonBytes.get());
            }

            InventorySync::instance().start([callbackLog](const int logLevel,
                                                          const char* tag,
                                                          const char* file,
                                                          const int line,
                                                          const char* func,
                                                          const char* logMessage,
                                                          va_list args)
                                            { callbackLog(logLevel, tag, file, line, func, logMessage, args); },
                                            configurationNlohmann);
        }
        catch (const std::exception& e)
        {
            logError(LOGGER_DEFAULT_TAG, "Error starting inventory sync: %s", e.what());
        }
    }

    void inventory_sync_stop()
    {
        InventorySync::instance().stop();
    }

#ifdef __cplusplus
}
#endif
