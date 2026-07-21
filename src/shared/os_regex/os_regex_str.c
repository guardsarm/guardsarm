/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "os_regex.h"
#include "os_regex_internal.h"


/* Check if a specific string is numeric (like "129544") */
int OS_StrIsNum(const char *str)
{
    if (str == NULL) {
        return (FALSE);
    }

    while (*str != '\0') {
        if (!_IsD(*str)) {
            return (FALSE);
        }
        str++;
    }

    return (TRUE);
}

/* Return the number of characters that both strings have in common */
size_t OS_StrHowClosedMatch(const char *str1, const char *str2)
{
    size_t count = 0;

    /* They don't match if any of them is null */
    if (!str1 || !str2) {
        return (0);
    }

    do {
        if (str1[count] != str2[count]) {
            break;
        }

        count++;
    } while ((str1[count] != '\0') && (str2[count] != '\0'));

    return (count);
}

