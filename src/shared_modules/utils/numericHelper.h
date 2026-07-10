/*
 * GuardSarm shared modules utils
 * Copyright (C) 2026 GuardSarm, Inc.
 * December 22, 2023.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef _NUMERIC_HELPER_H
#define _NUMERIC_HELPER_H

#include <iomanip>
#include <sstream>
#include <string>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"

namespace Utils
{
    static double floatToDoubleRound(const float number, const int precision)
    {
        std::stringstream ssAux;
        ssAux << std::fixed << std::setprecision(precision) << number;
        return std::stod(ssAux.str());
    }
} // namespace Utils

#pragma GCC diagnostic pop

#endif // _NUMERIC_HELPER_H
