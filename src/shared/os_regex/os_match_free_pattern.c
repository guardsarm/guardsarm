/* Copyright (C) 2026 GuardSarm, Inc.
 * All right reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "os_regex.h"
#include "os_regex_internal.h"
#include "shared.h"


/* Release all the memory created by the compilation/execution phases */
void OSMatch_FreePattern(OSMatch *reg)
{
    if(reg == NULL)
        return;

    /* Free the patterns */
    if (reg->patterns) {
        char **pattern = reg->patterns;
        while (*pattern) {
            os_free(*pattern);
            pattern++;
        }

        os_free(reg->patterns);
    }

    os_free(reg->size);
    os_free(reg->match_fp);
    os_free(reg->raw);

    return;
}
