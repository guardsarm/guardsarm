/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */


#ifndef ACLAPI_WRAPPERS_H
#define ACLAPI_WRAPPERS_H

#include <windows.h>
#include <accctrl.h>

#undef GetNamedSecurityInfo
#define GetNamedSecurityInfo wrap_GetNamedSecurityInfo
#undef SetNamedSecurityInfo
#define SetNamedSecurityInfo wrap_SetNamedSecurityInfo
#define GetSecurityInfo wrap_GetSecurityInfo

DWORD wrap_GetSecurityInfo(HANDLE handle,
                           SE_OBJECT_TYPE ObjectType,
                           SECURITY_INFORMATION SecurityInfo,
                           PSID *ppsidOwner,
                           PSID *ppsidGroup,
                           PACL *ppDacl,
                           PACL *ppSacl,
                           PSECURITY_DESCRIPTOR *ppSecurityDescriptor);

void expect_GetSecurityInfo_call(PSID ppsidOwner, PSID pSidGroup, DWORD ret_value);

DWORD wrap_GetNamedSecurityInfo(LPCSTR pObjectName,
                                SE_OBJECT_TYPE ObjectType,
                                SECURITY_INFORMATION SecurityInfo,
                                PSID *ppsidOwner,
                                PSID *ppsidGroup,
                                PACL *ppDacl,
                                PACL *ppSacl,
                                PSECURITY_DESCRIPTOR *ppSecurityDescriptor);

DWORD wrap_SetNamedSecurityInfo(LPSTR pObjectName,
                                SE_OBJECT_TYPE ObjectType,
                                SECURITY_INFORMATION SecurityInfo,
                                PSID psidOwner,
                                PSID psidGroup,
                                PACL pDacl,
                                PACL pSacl);

#endif
