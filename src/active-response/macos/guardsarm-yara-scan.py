#!/usr/bin/env python3
"""
GuardsArm on-agent YARA file scanner (Phase 7 — real file-content detection).

Uses the real libyara `yara` CLI to recursively scan watched directories against
the deployed rule pack, computes the SHA256 of each matched file, and emits an
`edr_malware` detection to the agent's EDR telemetry log (forwarded to the
manager, decoded to the `security` category). Deduplicates by rule:path so a
standing file alerts once. No Python dependencies (shells out to `yara`).

Run on a schedule (cron/systemd) or via the manager active-response scan trigger.
"""
import datetime
import hashlib
import json
import os
import platform
import subprocess
import sys


# --------------------------------------------------------------------------- #
# OS-aware defaults (pure helpers — unit-tested in test_yara_scan_osaware.py).
# Linux behavior is unchanged; macOS gets its own watched dirs and agent dir.
# --------------------------------------------------------------------------- #
def _is_macos():
    return sys.platform == "darwin" or platform.system() == "Darwin"


def _agent_dir(is_macos):
    """Agent install root. Linux: /var/gsmsec. macOS: /Library/Ossec when
    /var/gsmsec is absent (some macOS builds still ship under /var/gsmsec)."""
    if is_macos and not os.path.isdir("/var/gsmsec"):
        return "/Library/Ossec"
    return "/var/gsmsec"


def _default_watch_dirs(is_macos):
    """Default directories to scan when GS_YARA_DIRS is not set."""
    if is_macos:
        return ["/tmp", "/var/tmp", "/Users", "/Applications",
                "/Library/LaunchAgents", "/Library/LaunchDaemons"]
    return ["/tmp", "/var/tmp", "/dev/shm", "/home", "/root", "/var/www", "/srv"]


_MACOS = _is_macos()
_AGENT = _agent_dir(_MACOS)

RULES = os.environ.get("GS_YARA_RULES", os.path.join(_AGENT, "etc/yara/guardsarm-rules.yar"))
LOG = os.environ.get("GS_EDR_LOG", os.path.join(_AGENT, "logs/edr-telemetry.log"))
STATE = os.path.join(_AGENT, "var/yara-seen.json")
WATCH = os.environ.get("GS_YARA_DIRS", ":".join(_default_watch_dirs(_MACOS))).split(":")
SCAN_TIMEOUT = 180


def sha256(path):
    h = hashlib.sha256()
    try:
        with open(path, "rb") as fh:
            for chunk in iter(lambda: fh.read(65536), b""):
                h.update(chunk)
        return h.hexdigest()
    except Exception:
        return ""


def load_seen():
    try:
        with open(STATE) as f:
            return set(json.load(f))
    except Exception:
        return set()


def save_seen(s):
    try:
        os.makedirs(os.path.dirname(STATE), exist_ok=True)
        with open(STATE, "w") as f:
            json.dump(sorted(s)[-50000:], f)
    except Exception:
        pass


def scan():
    if not os.path.exists(RULES):
        return 0
    seen = load_seen()
    hits = 0
    with open(LOG, "a") as out:
        for d in WATCH:
            if not os.path.isdir(d):
                continue
            try:
                r = subprocess.run(["yara", "-r", "-w", "-f", "-N", RULES, d],
                                   capture_output=True, text=True, timeout=SCAN_TIMEOUT)
            except Exception:
                continue
            for line in r.stdout.splitlines():
                parts = line.split(" ", 1)               # "RULENAME /path/to/file"
                if len(parts) != 2:
                    continue
                rule, path = parts[0].strip(), parts[1].strip()
                key = f"{rule}:{path}"
                if key in seen:
                    continue
                seen.add(key)
                ev = {"collector": "edr_malware", "data": {"malware": {
                    "rule": rule, "file": path, "sha256": sha256(path),
                    "detected": datetime.datetime.now(datetime.timezone.utc).replace(tzinfo=None).isoformat() + "Z"}}}
                out.write(json.dumps(ev) + "\n")
                hits += 1
    save_seen(seen)
    return hits


if __name__ == "__main__":
    print(f"yara-scan: {scan()} new detection(s)")
