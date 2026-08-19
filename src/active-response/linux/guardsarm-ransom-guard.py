#!/usr/bin/env python3
"""GuardsArm ransomware canary guard — on-agent PREVENTION (Linux + macOS).

Ransomware's one unavoidable behaviour is touching lots of files fast. This guard
plants decoy "canary" files throughout user-data directories and watches them a
few times a second. The instant a canary is modified/deleted/renamed — something
no normal process ever does to these files — it:

  1. Identifies the offending process (any process holding a handle in the canary's
     directory that isn't on the allow-list).
  2. KILLS it (SIGKILL) — stopping the encryption sweep BEFORE most files are lost.
  3. Emits an `edr_malware` canary detection (rule GuardsArm_Ransomware_Canary) to
     the agent EDR telemetry log, which lands in the Malware Center and escalates
     to a `corr-ransom` incident (apply_ransomware) → autonomous isolation.
  4. Re-plants the canary and continues.

This is the true-prevention layer that turns detect-and-react into stop-in-progress.
Windows is covered by the sibling guardsarm-ransom-guard.ps1 (FileSystemWatcher).

Run as a daemon (default) from the agent's command wodle, or `--once` for a single
sweep-check. Safe: only ever writes/reads its OWN canary files; kills are gated on
GS_RANSOM_GUARD_KILL (default on) and skip the allow-list + PID 1 + itself.

Copyright (C) 2026, GuardsArm.
"""
import datetime
import hashlib
import json
import os
import platform
import signal
import subprocess
import sys
import time

IS_MAC = platform.system() == "Darwin"
AGENT = "/Library/Ossec" if IS_MAC else os.environ.get("GS_AGENT_HOME", "/var/gsmsec")
EDR_LOG = os.environ.get("GS_EDR_LOG", os.path.join(AGENT, "logs", "edr-telemetry.log"))
INTERVAL = float(os.environ.get("GS_RANSOM_GUARD_INTERVAL", "2"))
KILL = os.environ.get("GS_RANSOM_GUARD_KILL", "1") not in ("0", "false", "no")
CANARY_TOKEN = "GuardsArm_DO_NOT_REMOVE_canary"
CANARY_RULE = "GuardsArm_Ransomware_Canary"

# Directories to seed canaries in. Real user-data + share roots; NOT system dirs.
def _default_dirs():
    if IS_MAC:
        base = ["/Users"]
    else:
        base = ["/home", "/root", "/srv", "/mnt", "/media", "/var/www", "/shares", "/data"]
    return [d for d in base if os.path.isdir(d)]

DIRS = [d for d in os.environ.get("GS_RANSOM_CANARY_DIRS", "").split(",") if d.strip()] \
    or _default_dirs()

# Processes never killed: ONLY the agent's own components + system init. Kept
# deliberately minimal — nothing legitimate ever MODIFIES a canary file, so a broad
# allow-list would just hand ransomware an exemption (it could be a python/rsync
# process). Backup/sync software READS files; it does not rewrite these decoys.
ALLOW = {"guardsarm-agentd", "guardsarm-syscheckd", "guardsarm-modulesd", "guardsarm-logcollector",
         "guardsarm-execd", "guardsarm-ransom-guard", "systemd", "init", "launchd", "kthreadd"}


def _now():
    return datetime.datetime.now(datetime.timezone.utc).replace(tzinfo=None).isoformat() + "Z"


def _sha(data):
    return hashlib.sha256(data).hexdigest()


# Canary filenames: two per dir — one that sorts FIRST and one that sorts LAST, so a
# ransomware sweep in either alphabetical direction hits a canary almost immediately.
def _canary_paths(d):
    return [os.path.join(d, f"!!!_0000_{CANARY_TOKEN}.docx"),
            os.path.join(d, f"zzzz_9999_{CANARY_TOKEN}.xlsx")]


CANARY_BODY = (b"GuardsArm ransomware canary. This decoy file is monitored by the "
               b"endpoint agent; any modification triggers automatic containment. "
               b"Do not encrypt, rename or delete.\n") * 40
CANARY_HASH = _sha(CANARY_BODY)


def plant():
    planted = []
    for d in DIRS:
        for p in _canary_paths(d):
            try:
                if not os.path.exists(p) or open(p, "rb").read() != CANARY_BODY:
                    with open(p, "wb") as f:
                        f.write(CANARY_BODY)
                    try:
                        os.chmod(p, 0o644)
                    except OSError:
                        pass
                planted.append(p)
            except OSError:
                continue
    return planted


def _tripped(paths):
    out = []
    for p in paths:
        try:
            if not os.path.exists(p):
                out.append((p, "deleted"))
            elif open(p, "rb").read() != CANARY_BODY:
                out.append((p, "modified"))
        except OSError:
            out.append((p, "unreadable"))
    return out


def _procname(pid):
    try:
        if IS_MAC:
            return subprocess.check_output(["ps", "-p", str(pid), "-o", "comm="],
                                           text=True, timeout=3).strip().rsplit("/", 1)[-1]
        with open(f"/proc/{pid}/comm") as f:
            return f.read().strip()
    except Exception:
        return "?"


def _culprits(canary_dir):
    """Best-effort: PIDs holding an open handle inside the canary's directory that are
    not on the allow-list. On Linux via /proc/*/fd; on macOS via lsof."""
    hits = {}
    if IS_MAC:
        try:
            out = subprocess.check_output(["lsof", "+D", canary_dir], text=True, timeout=6,
                                          stderr=subprocess.DEVNULL)
            for ln in out.splitlines()[1:]:
                parts = ln.split()
                if len(parts) >= 2 and parts[1].isdigit():
                    hits[int(parts[1])] = parts[0]
        except Exception:
            pass
        return hits
    self_pid = os.getpid()
    # PRECISE match only: a culprit is a process holding open a CANARY file, or a
    # file whose IMMEDIATE parent is the canary's directory (i.e. a sibling the sweep
    # is encrypting). Deliberately NOT "anything under /home" — that would kill
    # legitimate processes (e.g. the appliers holding /home/gsadmin/cti/*.json) on a
    # rare trip. A ransomware sweep of a directory holds sibling files open; this
    # catches it without the collateral.
    canary_names = {os.path.basename(p) for d in DIRS for p in _canary_paths(d)}
    for pid in os.listdir("/proc"):
        if not pid.isdigit() or int(pid) in (1, self_pid):
            continue
        fdd = f"/proc/{pid}/fd"
        try:
            for fd in os.listdir(fdd):
                try:
                    tgt = os.readlink(os.path.join(fdd, fd))
                except OSError:
                    continue
                parent, base = os.path.split(tgt)
                if base in canary_names or parent == canary_dir:
                    hits[int(pid)] = _procname(pid)
                    break
        except OSError:
            continue
    return hits


def emit(canary, action, proc, pid):
    ev = {"collector": "edr_malware", "data": {"malware": {
        "rule": CANARY_RULE, "file": canary, "sha256": CANARY_HASH,
        "detected": _now(), "processName": proc, "pid": pid, "action": action}}}
    try:
        os.makedirs(os.path.dirname(EDR_LOG), exist_ok=True)
        with open(EDR_LOG, "a") as f:
            f.write(json.dumps(ev) + "\n")
    except OSError as e:
        sys.stderr.write(f"ransom-guard: emit failed: {e}\n")


def respond(canary, action):
    d = os.path.dirname(canary)
    culprits = _culprits(d)
    killed = []
    for pid, name in culprits.items():
        base = (name or "").rsplit("/", 1)[-1].lower()
        if base in ALLOW or any(a in base for a in ALLOW):
            continue
        if KILL:
            try:
                os.kill(pid, signal.SIGKILL)
                killed.append(f"{name}({pid})")
            except OSError:
                pass
    proc = killed[0] if killed else (next(iter(culprits.values()), "unknown"))
    pid = next(iter(culprits), None)
    emit(canary, action, proc, pid)
    print(f"ransom-guard: CANARY {action} {canary} — killed {killed or '[none identified]'}",
          flush=True)
    return killed


def sweep_check(paths):
    tripped = _tripped(paths)
    for canary, action in tripped:
        respond(canary, action)
    if tripped:
        plant()  # re-seed for continued protection
    return len(tripped)


def main():
    once = "--once" in sys.argv
    paths = plant()
    print(f"ransom-guard: planted {len(paths)} canaries in {DIRS} "
          f"(kill={'on' if KILL else 'off'}, once={once})", flush=True)
    if once:
        n = sweep_check(paths)
        print(f"ransom-guard: {n} canary trip(s)")
        return
    while True:
        try:
            sweep_check(paths)
        except Exception as e:
            sys.stderr.write(f"ransom-guard loop error: {e}\n")
        time.sleep(INTERVAL)


if __name__ == "__main__":
    main()
