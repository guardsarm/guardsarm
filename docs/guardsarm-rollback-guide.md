# GuardSarm — Rollback Guide

## R1 rollback is trivial by design

Release 1 changes only display strings and **adds** (never rewrites)
infrastructure. There is no data/state migration, so rollback never requires
data reversal.

### Option A — master switch (recommended)
`branding.json` is the single source of truth. Every shim
(`branding.h`, `branding.py`, `branding.sh`) falls back to the original Wazuh
values when branding.json is absent or empty.

```
rm branding.json      # or revert its values to Wazuh
# rebuild (C) / redeploy (Python, shell)
```

Result: help/version banners, packaging display (built via branding), and the
Python/shell surfaces revert to Wazuh. The C fallbacks live in
`src/shared/include/branding.h` (`#ifndef PRODUCT_* → "Wazuh"…`).

> Note: static display strings that were edited directly (README/docs prose,
> `version.rc`, WXS, DEB/RPM Description, systemd `Description=`, spec.yaml title,
> `NAME="GuardSarm"` in `shared.sh`) are reverted via git, not by removing
> branding.json — see Option B.

### Option B — per-phase git revert
Each phase was applied as an isolated, file-scoped change set. Any single phase
can be reverted independently (docs revert without touching packaging, etc.).
Because R1 touches no paths/users/services/schemas/protocol, reverting is safe on
running installs.

### Packaging rollback
Package IDs, MSI UpgradeCode, and service names are unchanged, so the package
manager treats a reverted build as a normal same-product upgrade/downgrade — no
orphaned services, no duplicate installs, no data loss.

### Pre-merge safety
The CI guard (`tools/rebrand/check-frozen-identifiers.sh`) blocks any change that
would touch a frozen identifier before it merges, so a compat regression is
normally caught before it ships — rollback for interop reasons should be rare.

## R2 rollback (when/if R2 is undertaken)

R2 renames real technical identifiers (paths, services, index names) and is
therefore genuinely harder to reverse. That is exactly why R2 is a **separate,
opt-in** release executed **one identifier class at a time**, each shipping with
its own reversible mechanism:

- **Paths:** bidirectional symlinks (`tools/migration/path-symlink.sh unlink …`).
- **Services:** alias units disabled; original unit retained.
- **Config:** dual-read loaders point back to the existing file.
- **Indices/fields:** dual-write + reindex keeping both names until cutover is
  proven; roll back by pointing reads at the legacy index.

No R2 identifier class is attempted until the Phase 5 tooling demonstrates a clean
forward-and-back on a snapshot install. See `guardsarm-remaining-work.md`.
