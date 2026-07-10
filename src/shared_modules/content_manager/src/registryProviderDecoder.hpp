/*
 * GuardSarm content manager
 * Copyright (C) 2026 GuardSarm, Inc.
 * March 25, 2023.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef _REGISTER_PROVIDER_DECODER_HPP
#define _REGISTER_PROVIDER_DECODER_HPP

#include "contentModuleFacade.hpp"
#include "dataDecoder.hpp"

/**
 * @brief RegisterProviderDecoder class.
 *
 */
class RegisterProviderDecoder final : public DataDecoder
{
private:
    const std::shared_ptr<std::vector<char>>& m_data;

public:
    /**
     * @brief Class constructor.
     *
     * @param data
     */
    explicit RegisterProviderDecoder(const std::shared_ptr<std::vector<char>>& data)
        : m_data {data}
    {
    }

    /**
     * @brief Decode data.
     *
     */
    void decode() override
    {
        auto decodedData = nlohmann::json::parse(m_data->begin() + sizeof(uint32_t), m_data->end());
        ContentModuleFacade::instance().addProvider(decodedData.at("name").get_ref<const std::string&>(),
                                                    decodedData.at("url").get_ref<const std::string&>(),
                                                    decodedData.at("path").get_ref<const std::string&>());
    }
};

#endif // _REGISTER_PROVIDER_DECODER_HPP
