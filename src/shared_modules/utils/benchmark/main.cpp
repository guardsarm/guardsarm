/*
 * GuardSarm - Content Migration tests
 * Copyright (C) 2015, Wazuh Inc.
 * Copyright (C) 2026, GuardSarm.
 * March 03, 2023.
 *
 */

#include <benchmark/benchmark.h>
#include <filesystem>

int main(int argc, char** argv)
{
    benchmark::Initialize(&argc, argv);
    if (::benchmark::ReportUnrecognizedArguments(argc, argv))
    {
        return EXIT_FAILURE;
    }
    benchmark::RunSpecifiedBenchmarks();
    benchmark::Shutdown();

    return EXIT_SUCCESS;
}
