/*
 * GuardSarm content manager
 * Copyright (C) 2026 GuardSarm, Inc.
 * March 25, 2023.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef _DATA_DECODER_HPP
#define _DATA_DECODER_HPP

/**
 * @brief DataDecoder class.
 *
 */
class DataDecoder
{
public:
    virtual ~DataDecoder() = default;

    /**
     * @brief Decodes data.
     *
     */
    virtual void decode() = 0;
};

#endif // _DATA_DECODER_HPP
