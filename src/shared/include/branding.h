/* Copyright (C) 2026 GuardSarm, Inc.
 * Copyright (C) 2026, GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

/* GuardSarm branding layer -- USER-VISIBLE, DISPLAY-ONLY values.
 *
 * The PRODUCT_* macros below carry the user-facing brand (product name, vendor,
 * contact, website and a display-only product version). They are injected at
 * build time from the repository-root branding.json by src/CMakeLists.txt,
 * mirroring the existing VERSION.json parse. If branding.json is absent, or a
 * field is empty, the #ifndef fallbacks here reproduce the original GuardSarm
 * branding -- so deleting branding.json and rebuilding is a complete rollback of
 * the user-visible rebrand.
 *
 * IMPORTANT -- COMPATIBILITY CONTRACT:
 * Do NOT use PRODUCT_* on the wire, in the agent `version` DB field, in
 * enrollment/keepalive parsing, in stored OS/version metadata, or in the
 * content-server HTTP User-Agent. Those are backward-compatibility contracts
 * with existing GuardSarm deployments and MUST keep using the frozen
 * __guardsarm_name / __guardsarm_version literals defined in defs.h. Load-bearing sites:
 *   src/shared/src/remoted_op.c        (agent keepalive version parsing)
 *   src/shared/src/guardsarmdb_queries_op.c (agent `version` DB field)
 *   src/shared/src/file_op.c           (OS/version metadata strings)
 *   src/shared_modules/content_manager/.../executionContext.hpp (HTTP User-Agent)
 * See docs/guardsarm-compatibility.md for the full frozen-identifier policy.
 */

#ifndef GUARDSARM_BRANDING_H
#define GUARDSARM_BRANDING_H

#ifndef PRODUCT_NAME
#define PRODUCT_NAME "GuardSarm"
#endif

#ifndef PRODUCT_AUTHOR
#define PRODUCT_AUTHOR "Wazuh Inc."
#endif

#ifndef PRODUCT_CONTACT
#define PRODUCT_CONTACT "info@guardsarmsiem.com"
#endif

#ifndef PRODUCT_SITE
#define PRODUCT_SITE "http://www.guardsarmsiem.com"
#endif

/* Display-only product version. When branding.json defines no product_version,
 * this falls back to the frozen __guardsarm_version banner value from defs.h. This
 * macro is for BANNERS/HELP output only -- never for on-wire version reporting. */
#ifndef PRODUCT_VERSION
#define PRODUCT_VERSION __guardsarm_version
#endif

#endif /* GUARDSARM_BRANDING_H */
