/* Copyright (C) 2026 GuardSarm, Inc.
 * Copyright (C) 2009 Trend Micro Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef H_MATHOP_OS
#define H_MATHOP_OS

/**
 * @brief Get the first available prime after the provided value.
 *
 * @param val Provided value.
 * @return Returns the first available prime or 0 on error.
 */
unsigned int os_getprime(unsigned int val);

#endif
