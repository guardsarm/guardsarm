# GuardSarm Rebrand — Inventory & Classification

Rebrand: **Wazuh → GuardSarm** (`guardsarm` / `GUARDSARM`). Repository:
Wazuh core monorepo, v5.0.1-alpha. Scale at discovery: **70,255** case-insensitive
occurrences of `wazuh` across **3,765** files.

## Classification key

| Cat | Meaning | R1 action |
|-----|---------|-----------|
| **A** | Safe user-visible branding | Rebrand now (via branding layer) |
| **B** | User-visible name with upgrade implications | Rebrand display; keep underlying id |
| **C** | Internal identifier needing backward compatibility | **Freeze** in R1; R2 w/ alias |
| **D** | Protocol/data/schema identifier | **Freeze** (do not change without dual-write/reindex) |
| **E** | Third-party / legal / historical | **Keep verbatim**; append GuardSarm |
| **F** | Generated artifact | Change the source, not the artifact |

## Per-component inventory (representative)

### Central definitions & build (src)
| Location | Identifier | Cat | Visibility | Sensitivity | Replacement | Migration | Verify |
|---|---|---|---|---|---|---|---|
| `src/shared/include/defs.h:70` | `__wazuh_name "Wazuh"` | **D** (split) | mixed | critical | Frozen literal for wire/DB; new `PRODUCT_NAME` for display | none (name split) | guard + interop keepalive test |
| `src/shared/include/defs.h:71` | `__wazuh_version "v5.0.1"` | **D** | user-visible | high | Frozen; display via `PRODUCT_VERSION` | none | User-Agent unchanged |
| `src/shared/include/defs.h:72-74` | `__author/__contact/__site` | A | user-visible | none | branding.json | none | `-V` output |
| `src/shared/include/defs.h:96,104` | `USER`/`GROUPGLOBAL "wazuh"` | C | internal | critical | Frozen (file ownership) | R2 useradd/alias | `id wazuh` |
| `src/shared/include/branding.h` (new) | `PRODUCT_*` | A | user-visible | none | GuardSarm | fallback→Wazuh | build `-DPRODUCT_*` |
| `VERSION.json` | version/stage | D | internal | critical | unchanged | none | build |
| `branding.json` (new) | display values | A | user-visible | none | GuardSarm | delete=rollback | CMake parse |
| `src/*/CMakeLists.txt` | `ARGV0="wazuh-*d"` | C | internal | critical | Frozen (process names) | R2 | `ps` |

### Core daemons, help & banners (src)
| Location | Identifier | Cat | Action |
|---|---|---|---|
| `src/shared/src/help.c` | banner via `__wazuh_*` | A | **Done** → `PRODUCT_*` |
| `src/wazuh_modules/src/main.c:162` | literal `"Wazuh Module Manager … Wazuh Inc."` | A | **Done** → `PRODUCT_*` (literal-sweep) |
| `src/addagent/src/main.c:48` | BANNER `__wazuh_name` | A | **Done** → `PRODUCT_*` |
| `src/util/src/verify-agent-conf.c:28` | `__wazuh_name` | A | **Done** → `PRODUCT_NAME` |
| `src/win32/win_agent.c:37` | `__wazuh_name/__wazuh_version` | A | **Done** → `PRODUCT_*` |
| `src/init/wazuh-server.sh`, `wazuh-client.sh` | "Starting Wazuh…" banners | A | **Done** → `$PRODUCT_NAME` via branding.sh; `DAEMONS`, `WAZUH_VERSION=` frozen |
| `src/shared/include/error_messages/*.h` | prose w/ "Wazuh" | B | Pattern: reword prose, keep codes + `WAZUH_*_HOME` names *(remaining)* |

### Load-bearing FROZEN `__wazuh_name`/`__wazuh_version` (do NOT rebrand)
| Location | Use | Cat |
|---|---|---|
| `src/shared/src/remoted_op.c:233` | `strstr(line,__wazuh_name)` keepalive version parse | D |
| `src/shared/src/wazuhdb_queries_op.c:409` | agent `version` DB field | D |
| `src/shared/src/file_op.c` (getuname ×5) | OS/version metadata reported+stored | D |
| `src/shared_modules/content_manager/.../executionContext.hpp:206` | HTTP User-Agent | D |
| `src/os_auth/src/auth.c`, `client-agent/src/start_agent.c`, `remoted/src/manager.c`, `shared/src/enrollment_op.c`, `wazuh_modules/.../wm_agent_upgrade_validate.c` | on-wire `__wazuh_version "v5.0.0"` + upgrade gating | D |

### Agent ↔ manager wire / enrollment (src)
| Identifier | Cat | Note |
|---|---|---|
| `OSSEC A:` / `OSSEC K:` / `OSSEC PASS:` | D | enrollment protocol prefixes |
| `#AES` / `:` crypto tokens | D | message encryption method |
| MSG_OVERHEAD (32/5/10/4), `%05hu%010u:%04u:` | D | message framing |
| ports 1514 / 1515 / 1516 | D | remoted / authd |
| `client.keys` `ID NAME IP KEY` | D | shared key format |
| cert subjects (`-S` param); test `CN=Wazuh` | D / E | prod parameterized; test fixture only |

### Data plane / indexer / engine (src)
| Identifier | Cat |
|---|---|
| `wazuh-states-*`, `wazuh-events-v5-*`, `wazuh-threatintel-*` index names | D |
| ECS fields `wazuh.agent.*`, `wazuh.cluster.*`, `wazuh.integration.*` | D |
| `indexer_connector/qa/**/template.json` | F (regenerate from source) |
| `.gitmodules` `wazuh-http-request` submodule | E (vendored) |

### API & framework
| Location | Identifier | Cat | Action |
|---|---|---|---|
| `api/api/spec/spec.yaml:44` | `title: 'Wazuh API REST'` | A | **Done** → GuardSarm |
| `api/api/spec/spec.yaml` (409/430 …) | `title: "Wazuh Error"` | D | Frozen (RFC7807 response field) |
| REST routes, response envelope, error codes, RBAC, `JWT_ISSUER='wazuh'`, port 55000 | D | Frozen |
| `framework/wazuh/` package name `wazuh` | C | Frozen (import contract) |
| `framework/wazuh/core/branding.py` (new) | display values | A | **Done** |
| `framework/setup.py`, `api/setup.py` | description/author/url/keywords | A/B | **Done** (name/pkg frozen) |
| cluster default name `wazuh`, delimiters, error codes | D | Frozen |

### Packaging & services
| Location | Identifier | Cat | Action |
|---|---|---|---|
| `src/win32/version.rc` | CompanyName/FileDescription/ProductName/Info | A | **Done** (versions kept, copyright appended) |
| `src/win32/wazuh-installer.wxs` | `ServiceInstall DisplayName/Description`, package `Description` | A | **Done** |
| `wazuh-installer.wxs` | `Name`/`Manufacturer` (feed `Software\[Manufacturer]\[ProductName]`) | C | **Frozen** (upgrade-dir registry path) |
| `wazuh-installer.wxs` | `WazuhSvc`, UpgradeCode GUID, `ApplicationFolderName`, OssecSvc migration | C | Frozen |
| `packages/debs/SPECS/*/debian/control` | Description/Maintainer/Homepage | A | **Done** (`Source`/`Package`/`Conflicts` frozen) |
| `packages/rpms/SPECS/*.spec` | Summary/%description/URL/Vendor/Packager | A | **Done** (`Name`/`%pre`/`%post`/users frozen) |
| `src/init/templates/wazuh-{agent,manager}.service` | `Description=` | A | **Done** (unit filename, ExecStart frozen) |
| DEB/RPM package IDs `wazuh-agent`/`wazuh-manager`, `com.wazuh.*` plist | C | Frozen |

### Config, ruleset, wodles, docs
| Location | Identifier | Cat | Action |
|---|---|---|---|
| `etc/templates/config/generic/header-comments.template` | header text/URLs | A | **Done** |
| `etc/**/ossec.conf`, XML tags `<ossec_config>`/`<wazuh_config>`, internal_options keys | C/D | Frozen |
| `ruleset/**` rule `<description>`/`<info>` prose | A/B | Reword-able *(remaining, low priority)* |
| `ruleset/**` rule IDs, decoder names, SCA policy/check IDs | D | Frozen |
| `wodles/**` output prose | A | *(remaining)*; AWS/GCP/Azure names E |
| `README.md`, `docs/book.toml`, `SECURITY.md`, `CONTRIBUTORS`, `install.sh` (URLs/banner) | A/E | **Done** (legal appended) |
Copyright (C) 2026 GuardSarm, Inc.

### CI / infra (.github)
| Identifier | Cat |
|---|---|
| `ghcr.io/wazuh`, S3 buckets, `packages(.dev).wazuh.com`, workflow `4_*`/`5_*` names, Coverity project | C/external (frozen; infra migration) |

## Remaining (pattern-documented, not yet applied in this pass)
- `error_messages/*.h`, `information_messages.h`, `warning_messages.h`: reword user-facing prose (keep numeric codes and `WAZUH_*_HOME` env-var names).
- `ruleset/**` rule/decoder `<description>`/`<info>` and SCA policy descriptions (large volume; alert-stream prose).
- `wodles/**` human-readable output strings.
- Full README body capability prose beyond the intro.
- Binary assets — see `guardsarm-asset-manifest.md`.
- Build-time parameterization of `version.rc`/WXS/spec display strings from `branding.json` (currently set as static GuardSarm text with source-of-truth noted).

All "Done" items were applied with the frozen-identifier guard
(`tools/rebrand/check-frozen-identifiers.sh`) passing before and after.
