/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#include "sddl_wrappers.h"
#include <stddef.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>

WINBOOL wrap_ConvertSidToStringSid(__UNUSED_PARAM(PSID Sid),
                                   LPSTR *StringSid) {
    *StringSid = mock_type(LPSTR);
    return mock();
}

void expect_ConvertSidToStringSid_call(LPSTR StringSid, int ret_value) {
    will_return(wrap_ConvertSidToStringSid, StringSid);
    will_return(wrap_ConvertSidToStringSid, ret_value);
}
