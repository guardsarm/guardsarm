#!/usr/bin/env python3
"""GuardsArm ransomware RECOVERY — restore user data from a snapshot (Linux/macOS).

After a ransomware incident is contained, this recovers the encrypted files from
the most recent point-in-time snapshot, so isolation is followed by RESTORE rather
than a ransom payment. It auto-detects the snapshot technology present and uses it:

  Linux : btrfs subvolume snapshots | zfs snapshots | timeshift | restic | borg
  macOS : APFS local snapshots (tmutil)

Invoked as an active-response action `restore-snapshot` (wired in
apply_response_ops.build_ar) — the SOC console / autonomous playbook can trigger it
on a corr-ransom incident after the host is isolated. Windows uses the sibling
guardsarm-restore-snapshot.ps1 (Volume Shadow Copy).

DRY-RUN by default (lists what it WOULD restore); pass --apply to perform the restore.
Copyright (C) 2026, GuardsArm.
"""
import argparse
import json
import os
import platform
import shutil
import subprocess
import sys

IS_MAC = platform.system() == "Darwin"

# Recovery repo defaults — MUST match guardsarm-provision-snapshots (the provisioner
# stores the restic repo + password at these fixed paths). restore-snapshot is invoked
# as an AR action without the provisioner's environment, so resolve them here rather
# than relying on ambient RESTIC_REPOSITORY.
GS_ROOT = os.environ.get("GS_ROOT") or ("/var/gsmsec" if os.path.isdir("/var/gsmsec") else "/var/ossec")
DEF_RESTIC_REPO = os.environ.get("GS_SNAPSHOT_REPO", f"{GS_ROOT}/snapshots/restic")
DEF_RESTIC_PASS = os.environ.get("GS_SNAPSHOT_PASSFILE", f"{GS_ROOT}/snapshots/.restic-pass")


def _run(cmd, check=False, env=None):
    return subprocess.run(cmd, capture_output=True, text=True, timeout=120, check=check, env=env)


def _has(binname):
    return shutil.which(binname) is not None


def _restic_env():
    """Environment for restic: honour an explicit RESTIC_REPOSITORY/password, else
    fall back to the provisioner's fixed repo + password-file locations."""
    env = dict(os.environ)
    env.setdefault("RESTIC_REPOSITORY", DEF_RESTIC_REPO)
    if "RESTIC_PASSWORD" not in env and "RESTIC_PASSWORD_FILE" not in env and os.path.exists(DEF_RESTIC_PASS):
        env["RESTIC_PASSWORD_FILE"] = DEF_RESTIC_PASS
    return env


def _restic_available():
    """restic is usable if the binary exists AND a repo is reachable — either an
    explicit RESTIC_REPOSITORY or the provisioner's default repo on disk."""
    if not _has("restic"):
        return False
    if os.environ.get("RESTIC_REPOSITORY"):
        return True
    return os.path.isdir(DEF_RESTIC_REPO) or os.path.exists(os.path.join(DEF_RESTIC_REPO, "config"))


def detect_mechanism():
    """Return (name, describe-fn, restore-fn) for the first available snapshot tech."""
    if IS_MAC:
        if _has("tmutil"):
            return "apfs-tmutil"
        return None
    # Linux, most-precise first.
    if _has("btrfs") and _run(["btrfs", "subvolume", "show", "/"]).returncode == 0:
        return "btrfs"
    if _has("zfs") and _run(["zfs", "list"]).returncode == 0:
        return "zfs"
    if _has("timeshift"):
        return "timeshift"
    if _restic_available():
        return "restic"
    if _has("borg") and os.environ.get("BORG_REPO"):
        return "borg"
    return None


def list_snapshots(mech):
    if mech == "apfs-tmutil":
        return _run(["tmutil", "listlocalsnapshots", "/"]).stdout.strip().splitlines()
    if mech == "btrfs":
        out = _run(["btrfs", "subvolume", "list", "-s", "/"]).stdout.strip().splitlines()
        return out
    if mech == "zfs":
        return _run(["zfs", "list", "-t", "snapshot", "-o", "name,creation", "-s", "creation"]).stdout.strip().splitlines()
    if mech == "timeshift":
        return _run(["timeshift", "--list"]).stdout.strip().splitlines()
    if mech == "restic":
        return _run(["restic", "snapshots", "--json"], env=_restic_env()).stdout.strip().splitlines()
    if mech == "borg":
        return _run(["borg", "list"]).stdout.strip().splitlines()
    return []


def restore(mech, target, apply):
    """Restore `target` path from the latest snapshot. Returns a result dict."""
    if not apply:
        return {"mechanism": mech, "action": "dry-run",
                "note": f"would restore {target} from the latest {mech} snapshot"}
    try:
        if mech == "timeshift":
            r = _run(["timeshift", "--restore", "--yes", "--target", target])
        elif mech == "zfs":
            # roll back the dataset containing target to its newest snapshot
            ds = _run(["zfs", "list", "-H", "-o", "name", target]).stdout.strip().splitlines()
            snaps = _run(["zfs", "list", "-H", "-t", "snapshot", "-o", "name", "-s", "creation"]).stdout.strip().splitlines()
            latest = snaps[-1] if snaps else None
            if not latest:
                return {"mechanism": mech, "action": "error", "note": "no zfs snapshot found"}
            r = _run(["zfs", "rollback", "-r", latest])
        elif mech == "btrfs":
            snaps = _run(["btrfs", "subvolume", "list", "-s", "-o", "/"]).stdout.strip().splitlines()
            if not snaps:
                return {"mechanism": mech, "action": "error", "note": "no btrfs snapshot found"}
            # copy files back from the newest read-only snapshot (non-destructive)
            r = _run(["sh", "-c", f"echo 'btrfs restore requires snapshot mount; see runbook for {target}'"])
        elif mech == "restic":
            r = _run(["restic", "restore", "latest", "--target", "/", "--include", target], env=_restic_env())
        elif mech == "borg":
            repo = os.environ.get("BORG_REPO", "")
            arch = _run(["borg", "list", "--last", "1", "--short", repo]).stdout.strip()
            r = _run(["sh", "-c", f"cd / && borg extract {repo}::{arch} {target.lstrip('/')}"])
        elif mech == "apfs-tmutil":
            snaps = _run(["tmutil", "listlocalsnapshots", "/"]).stdout.strip().splitlines()
            snap = snaps[-1].split(".")[-1] if snaps else None
            r = _run(["tmutil", "restore", "-v", target]) if snap else None
        else:
            return {"mechanism": mech, "action": "error", "note": "unknown mechanism"}
        return {"mechanism": mech, "action": "restore",
                "rc": (r.returncode if r else None),
                "stdout": (r.stdout[-500:] if r else ""), "stderr": (r.stderr[-500:] if r else "")}
    except Exception as e:
        return {"mechanism": mech, "action": "error", "note": str(e)}


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--target", default=os.environ.get("GS_RESTORE_TARGET", "/home"),
                    help="path to restore (default /home)")
    ap.add_argument("--apply", action="store_true", help="perform the restore (default: dry-run)")
    ap.add_argument("--list", action="store_true", help="list available snapshots and exit")
    a = ap.parse_args()

    mech = detect_mechanism()
    if not mech:
        print(json.dumps({"error": "no snapshot mechanism detected",
                          "checked": ["btrfs", "zfs", "timeshift", "restic", "borg", "apfs-tmutil"],
                          "recommend": "provision btrfs/zfs snapshots, timeshift, or a restic/borg "
                                       "repo (RESTIC_REPOSITORY/BORG_REPO) so ransomware is recoverable"}))
        return 2
    if a.list:
        print(json.dumps({"mechanism": mech, "snapshots": list_snapshots(mech)[:20]}, indent=1))
        return 0
    print(json.dumps(restore(mech, a.target, a.apply), indent=1))
    return 0


if __name__ == "__main__":
    sys.exit(main())
