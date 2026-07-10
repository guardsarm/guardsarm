/* Copyright (C) 2026 GuardSarm, Inc.
 * Copyright (C) 2009 Trend Micro Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

/* Help Function */

#include "shared.h"


void print_header()
{
    /* User-visible banner: uses the GuardSarm display branding (PRODUCT_*),
     * NOT the frozen __guardsarm_* wire/DB literals. */
    print_out(" ");
    print_out("%s %s - %s (%s)", PRODUCT_NAME, PRODUCT_VERSION, PRODUCT_AUTHOR, PRODUCT_CONTACT);
    print_out("%s", PRODUCT_SITE);
}

void print_version()
{
    print_out(" ");
    print_out("%s %s - %s", PRODUCT_NAME, PRODUCT_VERSION, PRODUCT_AUTHOR);
    print_out(" ");
    print_out("%s", __license);
    exit(0);
}
