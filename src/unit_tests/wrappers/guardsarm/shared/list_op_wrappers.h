/* Copyright (C) 2023, Wazuh Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */


#ifndef LIST_OP_WRAPPERS_H
#define LIST_OP_WRAPPERS_H

#include "shared.h"

void *__wrap_OSList_AddData(__attribute__((unused))OSList *list, __attribute__((unused))void *data);

void __wrap_OSList_DeleteThisNode(__attribute__((unused))OSList *list, __attribute__((unused))OSListNode *thisnode);

OSListNode *__wrap_OSList_GetFirstNode(__attribute__((unused))OSList *list);

void __wrap_OSList_Destroy(__attribute__((unused))OSList *list);

#endif // LIST_OP_WRAPPERS_H
