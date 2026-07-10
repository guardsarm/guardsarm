/*
 * Copyright (C) 2026 GuardSarm, Inc.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef __WMODULES_SCHEDULING_HELPERS_H__
#define __WMODULES_SCHEDULING_HELPERS_H__

#include "shared.h"
#include "wmodules.h"
#include <setjmp.h>
#include <cmocka.h>
#include <stdarg.h>
#include <stddef.h>

typedef struct test_structure
{
    gmodule* module;
    OS_XML xml;
    XML_NODE nodes;
} test_structure;

const XML_NODE string_to_xml_node(const char* string, OS_XML* _lxml);
sched_scan_config init_config_from_string(const char* string);

/* Sets current simulation time */
void set_current_time(time_t _time);

#endif
