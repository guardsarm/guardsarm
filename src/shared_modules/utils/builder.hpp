/*
 * GuardSarm shared modules utils
 * Copyright (C) 2026 GuardSarm, Inc.
 * January 19, 2022.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef _BUILDER_PATTERN_HPP
#define _BUILDER_PATTERN_HPP

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"

namespace Utils
{
    /**
     * @brief This class provides a simple interface to construct an object using a Builder pattern.
     *
     * @tparam T Type of the object to be built.
     * @tparam Ts Arguments.
     */
    template<typename T, class... Ts>
    class Builder
    {
    public:
        /**
         * @brief This method is used to build an object.
         *
         * @param args Arguments.
         * @return T Object built.
         */
        static T builder(Ts... args)
        {
            return T(args...); // Default constructor
        }

        /**
         * @brief This method returns a reference to the object.
         * @return T Reference to the object.
         */
        T& build()
        {
            return static_cast<T&>(*this); // Return reference to self
        }
    };
} // namespace Utils

#pragma GCC diagnostic pop

#endif // _BUILDER_PATTERN_HPP
