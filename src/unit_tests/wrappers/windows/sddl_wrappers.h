/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */


#ifndef SDDL_WRAPPERS_H
#define SDDL_WRAPPERS_H

#include <windows.h>

#undef  ConvertSidToStringSid
#define ConvertSidToStringSid wrap_ConvertSidToStringSid

WINBOOL wrap_ConvertSidToStringSid(PSID Sid, LPSTR *StringSid);

void expect_ConvertSidToStringSid_call(LPSTR StringSid, int ret_value);

#endif
