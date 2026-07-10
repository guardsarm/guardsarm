# GuardSarm — Compatibility Policy

GuardSarm is a rebrand of Wazuh that **must remain fully interoperable** with
existing Wazuh deployments. This document is the authoritative list of what is
**frozen** (kept as `wazuh`/`ossec`) versus what is **rebranded** (user-visible
display only), and why.

## Core guarantee (Release 1)

> An existing, unmodified Wazuh 5.0.x agent must enroll with and connect to a
> GuardSarm-branded manager, and an in-place upgrade of a Wazuh install to a
> GuardSarm-branded build must be a **display-only** change. R1 changes **no**
> paths, users, groups, service/daemon names, package IDs, index names, API
> contracts, or protocol constants.

This is enforced structurally: user-visible branding flows through the branding
layer (`branding.json` → `branding.h` / `branding.py` / `branding.sh`), while
every compatibility-sensitive identifier lives **outside** that layer and is
covered by the CI guard `tools/rebrand/check-frozen-identifiers.sh`.

## The `__wazuh_name` split (critical)

`__wazuh_name` is **not** purely cosmetic. It is used in load-bearing code:

- `remoted_op.c` — the manager finds an agent's version by `strstr(line, __wazuh_name)`
  on the keepalive line. If rebranded, a legacy Wazuh agent's line (containing
  "Wazuh vX.Y") no longer matches and its version is lost.
- `wazuhdb_queries_op.c` — builds the agent `version` DB field `"<__wazuh_name> <ver>"`,
  which is machine-readable and surfaced via the API.
- `file_op.c` (`getuname`) — composes OS/version metadata reported to the manager
  and stored in records.

Therefore:
- `__wazuh_name` (defs.h) stays **"Wazuh"**, frozen, used only at the sites above.
- A new **`PRODUCT_NAME`** (branding.h, from branding.json) carries the user-visible
  brand and is used by help/version banners and packaging.

Likewise `__wazuh_version` (defs.h `"v5.0.1"`; on-wire `"v5.0.0"` in protocol
files) is **frozen** — it feeds the content-server HTTP **User-Agent**
(`executionContext.hpp`) and upgrade version gating. The GuardSarm-specific
display version is a separate **`PRODUCT_VERSION`** (branding.json `product_version`).

## FROZEN in R1 (never blind-replaced)

**Wire / enrollment protocol** — `OSSEC A:`, `OSSEC K:`, `OSSEC PASS:` prefixes;
`#AES`/`:` crypto tokens; `MSG_OVERHEAD` (MD5=32, RANDOM=5, GLOBAL=10, LOCAL=4)
and the `%05hu%010u:%04u:` counter format; ports **1514** (remoted), **1515**
(authd), **1516** (cluster); `client.keys` `ID NAME IP KEY`; routing headers
`u:`/`s:`/`5:`. *Changing any breaks every deployed agent.*

**Filesystem / OS identity** — `/var/gsmsec` (agent), `/var/wazuh-manager`
(manager), `queue/sockets/*`, `etc/gsmsec.conf`, `etc/wazuh-manager.conf`,
`etc/client.keys`, `etc/shared`; users/groups **`wazuh`** and **`wazuh-manager`**.

**Process / service registry** — daemon `ARGV0` names (`wazuh-agentd`,
`wazuh-syscheckd`, `wazuh-manager-remoted`, …); control scripts (`wazuh-control`,
`wazuh-manager-control`); `WazuhSvc` (+ legacy `OssecSvc` migration retained);
`wazuh-agent.service` / `wazuh-manager.service`; `com.wazuh.*` plist; DEB/RPM
package IDs (`wazuh-agent`, `wazuh-manager`); **MSI UpgradeCode**
`F495AC57-7BDE-4C4B-92D8-DBE40A9AA5A0`; WXS `Name`/`Manufacturer` (feed the
`Software\[Manufacturer]\[ProductName]` upgrade-detection registry key).

**Data / API contracts** — OpenSearch index names `wazuh-states-*`,
`wazuh-events-v5-*`, `wazuh-threatintel-*`; ECS fields `wazuh.agent.*`,
`wazuh.cluster.*`, `wazuh.integration.*`; REST routes, response envelope fields
(`affected_items`, …), operationIds, error codes and RFC7807 `title`s; RBAC
catalog; `JWT_ISSUER='wazuh'`; API port **55000**; cluster default name `wazuh`,
delimiters and error codes; XML tags `<ossec_config>`/`<wazuh_config>`;
internal_options keys; SCA policy IDs and numeric check IDs; active-response
script basenames.

**CI / external infra** — `ghcr.io/wazuh`, S3 buckets, `packages(.dev).wazuh.com`,
workflow `4_*`/`5_*` names, Coverity project, `.gitmodules` submodule URL. Treated
as infrastructure migration; frozen in R1.

Copyright (C) 2026 GuardSarm, Inc.
`Copyright (C) 2009 Trend Micro Inc.`, OSSEC/Daniel Cid attribution, GPLv2 text,
OpenSSL license, `LICENSE`, and `CONTRIBUTORS` history is kept **verbatim**.
GuardSarm attribution is **appended** (see `NOTICE`), never substituted.

## Aliasing rule

A user-visible **label** may become "GuardSarm" while the **registered identifier**
stays Wazuh — e.g. `WazuhSvc` with `DisplayName="GuardSarm"`, or
`wazuh-manager.service` with `Description="GuardSarm manager"`. **Never invert**:
never change an identifier while keeping the Wazuh label.

## Enforcement

`tools/rebrand/check-frozen-identifiers.sh` asserts the presence of frozen
literals (protocol prefixes, ports, `__wazuh_name`/`__wazuh_version`, users/groups,
`WazuhSvc`, UpgradeCode, index prefixes) and that `LICENSE` and the canonical
copyright header are intact. Run it before and after every change and in CI.
