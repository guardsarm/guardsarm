# GuardSarm — Remaining Work (R2 design, cross-repo, external infra)

## Release strategy

**Release 1 (shippable now):** full user-visible GuardSarm branding + aliases +
inactive migration tooling. Every compat-sensitive identifier frozen. A stock
Wazuh agent connects to an R1 GuardSarm manager unchanged; Wazuh→R1 upgrade is
display-only. This is the interoperability-preserving milestone.

**Release 2 (optional, separate major release):** native identifier rename,
executed one class at a time behind the Phase 5 shims, each with dual-read/
dual-write + reindex + a full compat matrix and per-class go/no-go. **Default
recommendation: do not undertake R2 unless a concrete need arises.** Package IDs
and the MSI UpgradeCode should very likely **never** change (breaking the upgrade
chain outweighs cosmetic benefit).

## R2 design — per identifier class

Each class is independent; ship/skip individually.

1. **Filesystem paths** (`/var/wazuh-manager`, `/var/ossec`, config/log filenames)
   - Approach: install to the new path; create bidirectional symlinks
     (`tools/migration/path-symlink.sh`); dual-read config
     (`ossec.conf`/`wazuh-manager.conf` still honored). Touch `install.sh`,
     `src/init/shared.sh`/`inst-functions.sh`, `common.py` path constants,
     postinst/spec scriptlets.
   - Risk: file ownership, IPC socket paths, SELinux contexts. Test upgrade both ways.

2. **Users / groups** (`wazuh`, `wazuh-manager`)
   - Approach: keep existing user/group; optionally add GuardSarm-named alias via
     `useradd`/nsswitch; never `userdel` on upgrade. `defs.h` `-DUSER`/`-DGROUPGLOBAL`
     overrides + packaging `%pre`. Reassign file ownership only with a tested,
     idempotent migration.

3. **Daemon ARGV0 + control scripts** (`wazuh-agentd`, …, `wazuh-control`)
   - Approach: rename `ARGV0` in each `CMakeLists.txt`; update `DAEMONS` lists,
     PID/socket references, systemd `ExecStart`. Provide legacy control-script
     symlinks. Coordinate with monitoring that greps process names.

4. **Service names / plist** (`WazuhSvc`, `*.service`, `com.wazuh.*`)
   - Approach: install-time migration mirroring the existing `OssecSvc→WazuhSvc`
     logic (stop/delete old, register new), plus systemd `Alias=`. Keep MSI
     UpgradeCode.

5. **WXS `Manufacturer`/`ProductName`** (registry path
   `Software\[Manufacturer]\[ProductName]`)
   - Approach: add a RegistrySearch for the legacy key and migrate the stored
     install dir before switching. High risk to upgrade detection; validate on
     real Windows endpoints.

6. **Index names + ECS fields** (`wazuh-states-*`, `wazuh-events-v5-*`,
   `wazuh.*`)
   - Approach: coordinated **dual-write + reindex** with wazuh-dashboard and
     wazuh-indexer. The `wazuh-events-v5-*` version pattern already supports dual
     indexing during rollout. Never rename without reindex.

7. **Cluster default name `wazuh`, config keys, delimiters** — mixed-version
   cluster must form; freeze until a coordinated cluster upgrade path is tested.

8. **API** — routes, response envelope, error codes, `JWT_ISSUER`, RBAC catalog:
   only via a versioned `/v2`, never in place.

## Cross-repository checklist

This repo is the **core manager/agent**. A full-platform rebrand requires
coordinated work in the following repos (owner will clone/rebrand separately):

| Repo | What it carries | Coordination |
|---|---|---|
| **wazuh-dashboard** (Kibana fork) | The most user-visible surface: app title, login screen, favicons, logos, saved objects, plus consumers of API contract + index/ECS fields | Rebrand its own UI strings/assets; MUST coordinate before any R2 index/field rename |
| **wazuh-indexer** (OpenSearch fork) | Indexer branding, default user `wazuh-server`, security config | Rebrand display; index/user names frozen unless R2 reindex |
| **wazuh-agent-packages** | CI-referenced package build (`CI_WAZUH_AGENT_PACKAGES`) | Coordinate before workflow-name/namespace changes |
| **qa-integration-framework** | pip-installed test framework (`.github/actions/install_qa_framework`) | Keep URL until forked; branded tests need matching branch |
| **wazuh-qa** | Referenced in release-test issue template | Fork/branch alignment |
| Orchestration: **wazuh-docker, -ansible, -chef, -puppet, -kubernetes, -cloudformation, -bosh, -salt** | Deployment automation linked from README | Update links only after new-org repos exist |
| **wazuh-http-request** | Vendored git submodule (`.gitmodules`) | Do NOT change URL in R1; pin commit; rebrand only if rehosted |

## External infrastructure to provision (blocks URL/CI cutover completion)

The following were set to GuardSarm targets in R1 but require the destination to
exist, or were intentionally left as Wazuh (frozen) pending infra:

- **Domains/hosting:** `guardsarmsiem.com`, `documentation.guardsarmsiem.com` (README,
  book.toml, install.sh, header template, packaging URLs now point here).
- **GitHub org:** `github.com/guardsarm` (README/setup.py/book.toml
  `git-repository-url`) — repos must be created or links will 404.
- **Mailboxes:** `info@guardsarmsiem.com`, `security@guardsarmsiem.com` (SECURITY.md,
  setup.py, packaging) — must be provisioned.
- **Still frozen (Wazuh) pending infra migration:** `ghcr.io/wazuh` container
  registry, S3 buckets (`xdrsiem-packages-dev-internal`),
  `packages(.dev).wazuh.com`, Coverity project `wazuh-wazuh`, Docker Hub CI
  creds, WPK/package **signing keys** (new brand may need a new signing identity —
  security decision).
- **Social accounts** (Slack/Twitter/YouTube/LinkedIn) — removed from README top
  badges; re-add when GuardSarm accounts exist.

## Remaining in-repo prose sweeps (low risk, deferred by default)

- `src/shared/include/error_messages/*.h` — reword user-facing prose; keep numeric
  codes and `WAZUH_*_HOME` env-var names.
- `ruleset/**` rule `<description>`/`<info>` and SCA policy descriptions (large;
  affects the alert stream operators read daily). Rule/decoder/SCA **IDs** frozen.
- `wodles/**` human-readable output strings (AWS/GCP/Azure names stay).
- Full README body capability prose beyond the intro.
- Manager-generated **email/report templates** (if/where present) — inventory and
  rebrand logos/name; not located as first-class templates in this core repo.
- Build-time parameterization of `version.rc`/WXS/spec display fields from
  `branding.json` (currently static GuardSarm text with source-of-truth noted).

## Binary assets
See `guardsarm-asset-manifest.md`. Text branding cannot cover `.ico`/`.icns`/
installer bitmaps; real GuardSarm artwork is required before production.
