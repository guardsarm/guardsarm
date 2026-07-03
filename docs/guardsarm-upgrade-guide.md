# GuardSarm — Upgrade Guide (Release 1)

## Summary

Upgrading an existing **Wazuh 5.0.x** install to a **GuardSarm-branded (R1)**
build is a **display-only** change. R1 does not alter any path, user, group,
service name, daemon name, package ID, config filename, index name, API contract,
or protocol constant. No data migration is required or performed.

## What changes for the user
- Help/version output (`-h`, `-V`), startup/stop banners, and log product name
  read **GuardSarm** (display version `1.0.0` if `product_version` is set).
- Package Descriptions/Summaries, systemd unit `Description=`, Windows Services
  console `DisplayName`, `.exe` properties, installer text read **GuardSarm**.
- Documentation/website/contact strings point at `guardsarm.com` /
  `documentation.guardsarm.com` / `info@guardsarm.com` **where the destination
  infrastructure exists** (see remaining-work).

## What does NOT change (and why your deployment keeps working)
- **Agents:** existing Wazuh agents connect unchanged (ports 1514/1515, `OSSEC
  A/K/PASS` enrollment, `#AES` crypto, `client.keys`, keepalive version parse all
  frozen). No re-enrollment needed.
- **Filesystem:** `/var/ossec`, `/var/wazuh-manager`, `etc/ossec.conf`,
  `etc/wazuh-manager.conf`, `client.keys` unchanged; file ownership (`wazuh`,
  `wazuh-manager`) unchanged.
- **Services/packages:** `wazuh-agent`/`wazuh-manager` package IDs, `WazuhSvc`,
  `*.service` unit names, MSI UpgradeCode unchanged → normal in-place upgrade.
- **Data/API:** index names (`wazuh-states-*`, `wazuh-events-v5-*`), ECS fields,
  REST routes/response fields, RBAC, existing JWTs, saved objects unchanged.
- **Agent `version` records** stay `"Wazuh v…"` (frozen), so mixed fleets and
  historical records remain consistent.

## Upgrade steps (unchanged from Wazuh)
1. Back up as usual (config, `client.keys`, DB) — standard practice, not required
   by branding.
2. Install the GuardSarm-branded package over the existing one
   (`apt/yum` upgrade, or MSI over the same UpgradeCode). Package manager treats
   it as a normal same-product upgrade.
3. Restart services: `wazuh-control restart` / `wazuh-manager-control restart`
   (script names unchanged).
4. Verify (below).

## Verification matrix

| # | Check | Command / method | Expected |
|---|---|---|---|
| 1 | Frozen identifiers | `sh tools/rebrand/check-frozen-identifiers.sh` | PASS |
| 2 | Build wiring | CMake configure; inspect `compile_commands.json` | `-DPRODUCT_NAME="GuardSarm"`, `-DVERSION="v5.0.1"` unchanged |
| 3 | Banner | any daemon `-V` | `GuardSarm 1.0.0 - GuardSarm (info@guardsarm.com)` |
| 4 | Service display | `systemctl status wazuh-manager` / `services.msc` | Description/DisplayName = GuardSarm; unit/service **name** unchanged |
| 5 | **Legacy agent interop** | connect a stock Wazuh 5.0.x agent | enrolls (1515) + connects (1514); manager logs a populated **agent version** (keepalive parse) |
| 6 | DB version field | API `GET /agents` `version` | `"Wazuh v…"` (frozen) |
| 7 | API title | `GET /` | `title: GuardSarm API REST`; routes/fields unchanged |
| 8 | Package upgrade | `apt/yum` upgrade from prior; MSI over same UpgradeCode | success; `id wazuh`/`id wazuh-manager` intact |
| 9 | Indexer | existing `wazuh-states-*`/`wazuh-events-v5-*` | readable; events still routed |
| 10 | Legal | `git diff -- LICENSE`; copyright headers | no change |

## Environment-dependent tests (record if infra unavailable)
- Full C build requires the Linux toolchain (this repo builds via CMake/Make on
  Linux; a Windows dev host cannot produce Linux daemons). Record exact command
  (`cmake -S src -B build && make -C build`), required environment, and expected
  result; validate CMake branding parse where the full build is not available.
- Cross-version cluster, MSI upgrade on Windows endpoints, and dashboard render
  require their respective infrastructure.
