/* RSA-PKCS1-SHA256 signature
 * Copyright (C) 2026 GuardSarm, Inc.
 * July 3, 2017.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

// Unsign a WPK256 file, using a key path array. Returns 0 on success or -1 on error.
int w_wpk_unsign(const char * source, const char * target, const char ** ca_store);
