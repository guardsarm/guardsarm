/*
 * GuardSarm content manager
 * Copyright (C) 2026 GuardSarm, Inc.
 * March 25, 2023.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef _CONTENT_REGISTER_HPP
#define _CONTENT_REGISTER_HPP

#include "sharedDefs.hpp"
#include <json.hpp>
#include <memory>
#include <shared_mutex>
#include <string_view>
#include <thread>
#include <unordered_map>

#if __GNUC__ >= 4
#define EXPORTED __attribute__((visibility("default")))
#else
#define EXPORTED
#endif

/**
 * @brief ContentRegister class.
 *
 */
class EXPORTED ContentRegister final
{
private:
    std::string m_name;

public:
    /**
     * @brief Class constructor.
     *
     * @param topicName Topic name.
     * @param parameters Object parameters, including interval and ondemand.
     * @param fileProcessingCallback Callback function in charge of the file processing task.
     */
    explicit ContentRegister(std::string topicName,
                             const nlohmann::json& parameters,
                             FileProcessingCallback fileProcessingCallback);

    /**
     * @brief Destroy the Content Register object and cleanup
     *
     */
    ~ContentRegister();

    /**
     * @brief Changes schedular interval to a new value.
     *
     * @param newInterval New value to set.
     */
    void changeSchedulerInterval(size_t newInterval);
};

#endif // _CONTENT_REGISTER_HPP
