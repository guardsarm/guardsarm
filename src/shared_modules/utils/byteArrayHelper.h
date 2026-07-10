/*
 * GuardSarm shared modules utils
 * Copyright (C) 2026 GuardSarm, Inc.
 * March 15, 2021.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef _BYTE_ARRAY_HELPER_H
#define _BYTE_ARRAY_HELPER_H

#include <string>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"

namespace Utils
{
    static int32_t toInt32BE(uint8_t const* bytes)
    {
        return static_cast<int32_t>(bytes[3]) |
               static_cast<int32_t>(bytes[2]) << 8 |
               static_cast<int32_t>(bytes[1]) << 16 |
               static_cast<int32_t>(bytes[0]) << 24;
    }

    static int32_t toInt32LE(uint8_t const* bytes)
    {
        return static_cast<int32_t>(bytes[0]) |
               static_cast<int32_t>(bytes[1]) << 8 |
               static_cast<int32_t>(bytes[2]) << 16 |
               static_cast<int32_t>(bytes[3]) << 24;
    }
}

#pragma GCC diagnostic pop

#endif // _BYTE_ARRAY_HELPER_H
