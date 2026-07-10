# GuardSarm migration tooling (Release 2 enablement — INACTIVE in R1)

This directory contains **opt-in, inactive** tooling that prepares operators for a
future Release 2 (R2) in which selected *technical* identifiers may be renamed
from `guardsarm`/`ossec` to GuardSarm-native names. **None of this runs
automatically.** Release 1 changes only user-visible branding and keeps every
technical identifier frozen (see `docs/guardsarm-compatibility.md`), so R1
upgrades are display-only and require no migration.

These scripts are shipped in R1 so the R2 path can be trialed on snapshots before
any native rename is ever attempted. They are deliberately conservative:
idempotent, reversible, and non-destructive. They never delete user data.

## Contents

| File | Purpose | State |
|------|---------|-------|
| `path-symlink.sh`   | Create bidirectional symlinks between legacy and GuardSarm install paths (e.g. `/var/guardsarm-manager` <-> `/var/guardsarm-manager`). | stub / inactive |
| `service-alias.md`  | Design + template for systemd/launchd/Windows service alias units so both legacy and new service names resolve. | design |
| `dual-read-config.md` | Design for loaders that accept both `gsmsec.conf`/`guardsarm-manager.conf` and any future GuardSarm-named config, preferring the existing file. | design |

## Non-negotiable rules for any R2 migration

1. **Never delete or overwrite user data.** Migrations create links/copies and
   leave originals in place.
2. **Idempotent.** Safe to re-run; detect already-migrated state and no-op.
3. **Reversible.** Every step has a documented rollback (remove the symlink,
   disable the alias unit, point the loader back).
4. **One identifier class at a time.** Paths, then users/groups, then service
   names, etc. — each behind its own go/no-go with a compat matrix.
5. **Protocol/data identifiers are out of scope even for R2** unless a coordinated
   dual-write + reindex is implemented with the dashboard/indexer teams.

See `docs/guardsarm-remaining-work.md` for the full R2 design and the
cross-repository checklist.
