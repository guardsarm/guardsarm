/*
 * GuardSarm keystore
 * Copyright (C) 2026 GuardSarm, Inc.
 * January 24, 2024.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef _KEYSTORE_HPP
#define _KEYSTORE_HPP

#include <string>

class Keystore final
{

public:
    Keystore() = default;

    /**
     * Insert or update a key-value pair in the specified column family.
     *
     * @param columnFamily The target column family.
     * @param key The key to be inserted or updated.
     * @param value The corresponding value.
     */
    static void put(const std::string& columnFamily, const std::string& key, const std::string& value);

    /**
     * Get the key value in the specified column family.
     *
     * @param columnFamily The target column family.
     * @param key The key to be inserted or updated.
     * @param value The corresponding value to be returned.
     */
    static void get(const std::string& columnFamily, const std::string& key, std::string& value);

    /**
     * Get the key value in the specified column family.
     *
     * @param columnFamily The target column family.
     * @param key The key to be inserted or updated.
     * @return The corresponding value to be returned.
     */
    static std::string get(const std::string& columnFamily, const std::string& key);
};

#endif // _KEYSTORE_HPP
