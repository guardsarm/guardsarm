/*
 * GuardSarm shared modules utils
 * Copyright (C) 2026 GuardSarm, Inc.
 * Apr 24, 2023.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef LOGGER_HELPER_TEST_H
#define LOGGER_HELPER_TEST_H

#include "loggerHelper.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

void debugVerboseTestFunction(const char* tag, const char* file, int line, const char* func, const char* msg);
void debugTestFunction(const char* tag, const char* file, int line, const char* func, const char* msg);
void infoTestFunction(const char* tag, const char* file, int line, const char* func, const char* msg);
void warningTestFunction(const char* tag, const char* file, int line, const char* func, const char* msg);
void errorTestFunction(const char* tag, const char* file, int line, const char* func, const char* msg);
void logFunctionWrapper(
    int level, const char* tag, const char* file, int line, const char* func, const char* msg, va_list args);
std::stringstream ssOutput;

class LoggerHelperTest : public ::testing::Test
{
protected:
    LoggerHelperTest() = default;
    virtual ~LoggerHelperTest() = default;

    static void SetUpTestSuite()
    {
        Log::assignLogFunction(
            [](const int logLevel,
               const char* tag,
               const char* file,
               const int line,
               const char* func,
               const char* logMessage,
               va_list args)
            { logFunctionWrapper(logLevel, tag, file, line, func, logMessage, args); });
    }

    virtual void SetUp()
    {
        ssOutput.str("");
    }
};
#endif // LOGGER_HELPER_TEST_H
