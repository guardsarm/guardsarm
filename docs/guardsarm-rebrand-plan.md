# GuardSarm Rebrand — Plan

Companion to `guardsarm-rebrand-inventory.md` (what) and
`guardsarm-compatibility.md` (frozen policy). This file is the architecture map,
risk register, phasing, and release/rollback strategy.

## Architecture / component map

The Wazuh brand flows through five layers:

1. **C/C++ core** (`src/`) — anchored by `src/shared/include/defs.h` marketing
   macros; consumed by `src/shared/src/help.c`. Version from `VERSION.json` via
   `src/CMakeLists.txt` (`-DVERSION`/`-DREVISION`). Daemon identity via per-component
   `ARGV0`. `USER`/`GROUPGLOBAL`/`WAZUH_HOME_ENV` are `#ifndef`-guarded.
2. **Python framework + API + wodles** — import package literally `wazuh`; display
   strings, API title, `JWT_ISSUER`, paths scattered across `core/common.py`,
   `authentication.py`, `setup.py`.
3. **Shell / init** (`src/init/*.sh`, `install.sh`) — banner variables + DAEMONS
   lists + INSTALLDIR paths + doc URLs.
4. **Packaging** (`packages/`, `src/win32/`) — DEB control, RPM spec, WXS, macOS,
   systemd units. Split between display fields and compat-critical identity.
5. **Data plane + integration contracts** — OpenSearch index names, ECS schema,
   REST routes, RBAC, cluster protocol, wire prefixes, ports, SCA IDs.

**Choke point introduced:** repo-root `branding.json` → generated per-language
shims (`branding.h`, `branding.py`, `branding.sh`). One edit + rebuild flips every
user-visible surface; every compat-sensitive identifier sits physically outside
the layer.

## Branding layer

`branding.json` (display-only: product_name/id/upper, product_version, vendor,
author, contact, security_contact, site, doc_url, github_org). It defines **no**
paths, users, daemons, ports, index names, or protocol constants.

- **C/C++**: `src/CMakeLists.txt` parses branding.json (mirroring the VERSION.json
  parse) → `-DPRODUCT_NAME/…`. `branding.h` provides `#ifndef` fallbacks to the
  original Wazuh values, so an absent branding.json = clean rollback. `__wazuh_name`
  is **split** from `PRODUCT_NAME`.
- **Python**: `framework/wazuh/core/branding.py` resolves branding.json (env →
  walk-up → WAZUH_PATH), Wazuh fallbacks.
- **Shell**: `src/init/branding.sh` sourced by control/install scripts.
- **Packaging**: display fields set to GuardSarm; identifiers frozen. (Build-time
  parameterization from branding.json is a documented follow-up.)

## Risk register

| Risk | Severity | Mitigation |
|---|---|---|
| Bulk replace hits a wire constant or a load-bearing `__wazuh_name`/`__wazuh_version` site → breaks all deployed agents | critical | No global replace; name/version split; CI guard covers those exact files; legacy-keepalive interop test |
| Rename `WazuhSvc` / MSI UpgradeCode / WXS `Manufacturer`+`ProductName` → breaks Windows upgrade & service mgmt | critical | Only DisplayName/Description/package Description parameterized; identifiers + registry-path fields frozen; OssecSvc migration retained |
| Rename index names / ECS fields → breaks routing, dashboards, existing indices | critical | Frozen in R1; R2 only via versioned dual-write + reindex coordinated with dashboard/indexer |
| Change paths / users / groups → breaks ownership, IPC, upgrade | critical | Frozen R1; R2 behind symlink/alias tooling with tested upgrade |
| Alter API contract (routes, envelope, `JWT_ISSUER`, RBAC, error codes) → breaks clients, invalidates JWTs | critical | Branding layer never touches API identifiers; future change requires /v2 |
| Strip/replace copyright / GPLv2 / OSSEC attribution → license violation | high | Category E; guard checks LICENSE + header; append-only via NOTICE |
| Mixed-version/name cluster fails to form | high | Cluster identifiers frozen; cross-version test before any R2 |
| URLs cut over before infra exists → dead links | medium | Isolated; provided guardsarm.com targets documented as requiring provisioning |
| CI breakage from `ghcr.io/wazuh`/S3/workflow renames | high | Frozen R1; external-infra migration |
| SCA/AR IDs renamed → breaks compliance history/rule refs | high | Frozen (category D) |
| Vendored `http-request` submodule drift | medium | `.gitmodules` untouched; pin commit |

## Ordered phases (R1) — status

| Phase | Goal | Status |
|---|---|---|
| 0 | Branding layer scaffolding + `__wazuh_name`/version split | **done** |
| 0.5 | Frozen-identifier CI guard | **done** |
| 1 | Docs & prose (README, NOTICE, CONTRIBUTORS, book.toml, SECURITY) | **done** |
| 2 | Help/version output, log banners, C literal sweep | **done** |
| 3 | Packaging display fields (version.rc, WXS, DEB/RPM, systemd) | **done** |
| 4 | Config comments & API metadata (spec title, setup.py, header template) | **done** |
| 5 | R2 tooling (inactive) + asset manifest | **done** |
| 6 | External URL cutover (install.sh, SECURITY, book.toml) | **done** (targets require infra) |
| — | Remaining prose sweeps (error msgs, ruleset, wodles) | pattern-documented |

## Testing strategy
See `guardsarm-upgrade-guide.md` (verification matrix). Primary gate: the CI
guard + a legacy-agent-→-branded-manager interop test asserting the keepalive
version parse still populates `agent version` and the DB `version` field stays
`"Wazuh v…"`.

## Rollback / Release
See `guardsarm-rollback-guide.md` and the "Release strategy" in
`guardsarm-remaining-work.md` (R1 shippable; R2 optional, per-identifier-class).

## Open decisions (carried)
- Exact legal entity string ("GuardSarm, Inc." placeholder) to confirm.
- Whether/which R2 identifier classes to execute (default: none).
- Package IDs + MSI UpgradeCode: recommend **never** change.
