/* Copyright (C) 2015, Wazuh Inc.
 * Copyright (C) 2009 Trend Micro Inc.
 * All rights reserved.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation
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
