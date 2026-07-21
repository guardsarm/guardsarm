/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
*/

#include "shared.h"


unsigned int os_getprime(unsigned int val)
{
    unsigned int i;
    unsigned int max_i;

    /* Value can't be even */
    if ((val % 2) == 0) {
        val++;
    }

    do {
        /* We just need to check odd numbers up until half
         * the size of the provided value
         */
        i = 3;
        max_i = val / 2;
        while (i <= max_i) {
            /* Not prime */
            if ((val % i) == 0) {
                break;
            }
            i += 2;
        }

        /* Prime */
        if (i >= max_i) {
            return (val);
        }
    } while (val += 2);

    return (0);
}
