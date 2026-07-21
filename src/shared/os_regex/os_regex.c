/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "os_regex.h"
#include "shared.h"


/*  This function is a wrapper around the compile/execute
 *  functions. It should only be used when the pattern is
 *  only going to be used once.
 *  Returns 1 on success or 0 on failure.
 */
int OS_Regex(const char *pattern, const char *str)
{
    int r_code = 0;
    OSRegex reg;

    /* If the compilation failed, we don't need to free anything */
    if (OSRegex_Compile(pattern, &reg, 0)) {
        if (OSRegex_Execute(str, &reg)) {
            r_code = 1;
        }

        OSRegex_FreePattern(&reg);
    }

    return (r_code);
}


void OSRegex_free_regex_matching (regex_matching *reg) {

    if (!reg) {
        return;
    }

    if (reg->sub_strings) {
        for (unsigned int i = 0; reg->sub_strings[i]; i++) {
            os_free(reg->sub_strings[i]);
        }
        os_free(reg->sub_strings);
    }

     if (reg->prts_str) {
        for (unsigned int i = 0; reg->d_size.prts_str_size[i] && reg->prts_str[i]; i++) {
            os_free(reg->prts_str[i]);
        }
        os_free(reg->prts_str);
    }
    os_free(reg->d_size.prts_str_size);
}
