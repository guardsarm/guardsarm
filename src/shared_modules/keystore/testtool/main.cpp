/*
 * GuardSarm keystore
 * Copyright (C) 2026 GuardSarm, Inc.
 * July 1, 2024.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#include "keyStore.hpp"
#include "testtoolArgsParser.hpp"
#include <functional>
#include <iostream>

int main(int argc, char* argv[])
{
    try
    {
        CmdLineArgs args(argc, argv);
        std::string key = args.getKey();
        std::string column = args.getColumnFamily();
        std::string value = args.getValue();

        if (value.empty())
        {
            Keystore::get(column, key, value);
            std::cout << value << std::endl;
        }
        else
        {
            Keystore::put(column, key, value);
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}
