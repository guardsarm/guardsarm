/*
 * Copyright (C) 2026 GuardSarm, Inc.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 *
 * */

#include <stddef.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>
#include <plist/plist.h>
#include "../common.h"
#include "shared.h"

void wrap_plist_from_bin (char * bin, size_t size, plist_t *node) {
    if (test_mode) {
        check_expected(bin);
        *node = mock_type(plist_t);
        return;
    }
    
    plist_from_bin(bin, size, node);
}

void wrap_plist_to_xml (plist_t *node, char ** xml, uint32_t *size) {
    if (test_mode) {
        check_expected(node);
        char *tmp = mock_type(char*);
        w_strdup(tmp, *xml);
        *size = mock_type(uint32_t);
        return;
    }
    
    plist_to_xml(node, xml, size);
}

void wrap_plist_free(plist_t node) {
    if (test_mode) {
        check_expected(node);
        return;
    }
    
    plist_free(node);
}