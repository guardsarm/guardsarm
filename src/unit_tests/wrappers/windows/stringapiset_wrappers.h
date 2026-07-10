
/*
 * Copyright (C) 2026 GuardSarm, Inc.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */
#ifndef STRINGAPISET_WRAPPERS_H
#define STRINGAPISET_WRAPPERS_H

#include <windows.h>

#define WideCharToMultiByte wrap_WideCharToMultiByte

int wrap_WideCharToMultiByte(UINT CodePage,
                             DWORD dwFlags,
                             LPCWCH lpWideCharStr,
                             int cchWideChar,
                             LPSTR lpMultiByteStr,
                             int cbMultiByte,
                             LPCCH lpDefaultChar,
                             LPBOOL lpUsedDefaultChar);


#endif
