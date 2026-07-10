/*
 * GuardSarm Syscheck
 * Copyright (C) 2026 GuardSarm, Inc.
 * September 6, 2021.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef DB_COMMONDEFS_H
#define DB_COMMONDEFS_H
#include "commonDefs.h"
#include "logging_helper.h"

#define FIMDB_FILE_TABLE_NAME "file_entry"
#define FIMDB_FILE_TXN_TABLE  "{\"table\": \"file_entry\"}"
#define FILE_PRIMARY_KEY      "path"

#define FIMDB_REGISTRY_KEY_TABLENAME   "registry_key"
#define FIMDB_REGISTRY_KEY_TXN_TABLE   "{\"table\": \"registry_key\"}"
#define FIMDB_REGISTRY_VALUE_TABLENAME "registry_data"
#define FIMDB_REGISTRY_VALUE_TXN_TABLE "{\"table\": \"registry_data\"}"

typedef enum FIMDBErrorCode
{
    FIMDB_OK = 0,
    FIMDB_ERR = -1,
    FIMDB_FULL = -2
} FIMDBErrorCode;

typedef void((*logging_callback_t)(const modules_log_level_t level, const char* log));
typedef void((*callback_t)(void* return_data, void* user_data));
typedef void((*callback_txn_t)(ReturnTypeCallback result_type, const cJSON* result_json, void* user_data));

/**
 * @brief callback context.
 */
typedef struct
{
    union
    {
        callback_t callback;
        callback_txn_t callback_txn;
    };
    void* context;
} callback_context_t;

enum OSType
{
    OTHERS,
    WINDOWS
};

#endif // DB_COMMONDEFS_H
