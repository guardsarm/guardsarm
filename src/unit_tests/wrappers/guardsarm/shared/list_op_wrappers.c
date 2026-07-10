/* Copyright (C) 2023, Wazuh Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#include "list_op_wrappers.h"
#include <stddef.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>

void *__wrap_OSList_AddData(__attribute__((unused))OSList *list, __attribute__((unused))void *data) {

    bool must_free = mock();
    if (must_free) {
        os_free(data);
    }
    return mock_type(void *);
}

void __wrap_OSList_DeleteThisNode(__attribute__((unused))OSList *list, __attribute__((unused))OSListNode *thisnode) {
    function_called();
    return;
}

OSListNode *__wrap_OSList_GetFirstNode(__attribute__((unused))OSList *list) {
    return mock_type(OSListNode *);
}

extern void __real_OSList_Destroy(__attribute__((unused))OSList *list);
void __wrap_OSList_Destroy(__attribute__((unused))OSList *list) {
    if (test_mode) {
        function_called();
    }
    else {
        __real_OSList_Destroy(list);
    }
    return;
}
