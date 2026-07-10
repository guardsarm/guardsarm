/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#include "zlib_wrappers.h"
#include <stddef.h>
#include <stdarg.h>
#include <setjmp.h>
#include <stdint.h>
#include <cmocka.h>
#include <string.h>


int __wrap_gzread(gzFile gz_fd,
                  void* buf,
                  int len) {
    check_expected_ptr(gz_fd);
    int n = mock();
    if(n <= len && (n > 0)) {
        memcpy(buf, mock_type(void*), n);
    }
    return n;
}

gzFile __wrap_gzopen(const char * path,
                     const char * mode) {
    check_expected(path);
    check_expected(mode);

    return mock_type(gzFile);
}

int __wrap_gzclose(gzFile file) {
    check_expected_ptr(file);
    return mock();
}

int __wrap_gzeof(gzFile file) {
    check_expected_ptr(file);
    return mock();
}

const char * __wrap_gzerror(gzFile file,
                            int *errnum) {
    check_expected_ptr(file);
    *errnum = mock();
    return mock_type(char*);
}

int __wrap_gzwrite(gzFile file,
                   voidpc buf,
                   unsigned int len) {
    check_expected_ptr(file);
    check_expected(buf);
    check_expected(len);
    return mock();

}
