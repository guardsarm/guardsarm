/*
 * Copyright (C) 2026 GuardSarm, Inc.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */
#include "stringapiset_wrappers.h"
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

int wrap_WideCharToMultiByte(__UNUSED_PARAM(UINT CodePage),
                             __UNUSED_PARAM(DWORD dwFlags),
                             LPCWCH lpWideCharStr,
                             int cchWideChar,
                             LPSTR lpMultiByteStr,
                             int cbMultiByte,
                             __UNUSED_PARAM(LPCCH lpDefaultChar),
                             __UNUSED_PARAM(LPBOOL lpUsedDefaultChar)) {
    check_expected(lpWideCharStr);
    check_expected(cchWideChar);

    if(lpMultiByteStr)
        strncpy(lpMultiByteStr, mock_type(char*), cbMultiByte);

    return mock();
}
