/*
 * GuardSarm Inventory sync
 * Copyright (C) 2026 GuardSarm, Inc.
 * May 14, 2025.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef _INVENTORY_SYNC_HPP
#define _INVENTORY_SYNC_HPP

#if __GNUC__ >= 4
#define EXPORTED __attribute__((visibility("default")))
#else
#define EXPORTED
#endif

#include "json.hpp"
#include "singleton.hpp"
#include <functional>
#include <string>

/**
 * @brief InventorySync class.
 *
 */
class EXPORTED InventorySync final : public Singleton<InventorySync>
{
public:
    /**
     * @brief Starts Inventory Sync.
     *
     * @param logFunction Log function to be used.
     * @param configuration Sync configuration.
     */
    void
    start(const std::function<void(const int, const char*, const char*, const int, const char*, const char*, va_list)>&
              logFunction,
          const nlohmann::json& configuration) const;
    /**
     * @brief Stops Inventory sync.
     *
     */
    void stop() const;
};

#endif // _INVENTORY_SYNC_HPP
