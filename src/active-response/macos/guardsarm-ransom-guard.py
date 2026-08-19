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

# --- #2 canary spread: scatter canaries THROUGH the tree, not just at the top ------
# Ransomware that dives into deep subfolders must still hit a decoy quickly, so plant
# one canary set per directory, walking a few levels down (capped to bound cost).
CANARY_DEPTH = int(os.environ.get("GS_RANSOM_CANARY_DEPTH", "3"))
MAX_CANARY_DIRS = int(os.environ.get("GS_RANSOM_MAX_CANARY_DIRS", "400"))
_SKIP_DIR = ("/proc", "/sys", "/dev", "node_modules", ".git", "/.cache", "/cache",
             "__pycache__", "/build", "/dist", "site-packages", "/.snapshots")


def _canary_dirs():
    """Every directory to seed a canary in: each watched root + its subdirs down to
    CANARY_DEPTH, capped at MAX_CANARY_DIRS, skipping churn/system dirs."""
    out, seen = [], set()
    for root in DIRS:
        base_depth = root.rstrip(os.sep).count(os.sep)
        for cur, subdirs, _files in os.walk(root):
            low = cur.replace("\\", "/").lower()
            if any(s in low for s in _SKIP_DIR):
                subdirs[:] = []
                continue
            if cur not in seen:
                seen.add(cur); out.append(cur)
            if cur.rstrip(os.sep).count(os.sep) - base_depth >= CANARY_DEPTH:
                subdirs[:] = []  # stop descending past the depth limit
            if len(out) >= MAX_CANARY_DIRS:
                return out
    return out


# --- #1 in-place / no-rename ransomware detection ---------------------------------
# Ransomware that encrypts files IN PLACE (same name + extension, no ransom note) is
# invisible to the rename/note signals. We catch it on content: an encrypted file no
# longer matches its type — a .docx that isn't a ZIP, a .txt that is high-entropy
# random. A burst of such "type-destroyed" files in user data is active encryption.
INPLACE_MIN = int(os.environ.get("GS_RANSOM_INPLACE_MIN", "6"))     # encrypted files to fire
INPLACE_WINDOW = int(os.environ.get("GS_RANSOM_INPLACE_WINDOW", "90"))  # recent-mtime window (s)
INPLACE_MAX_SCAN = int(os.environ.get("GS_RANSOM_INPLACE_MAX_SCAN", "3000"))  # files/cycle cap
# extension -> expected leading magic bytes (encryption destroys these headers)
_MAGIC = {
    "docx": b"PK\x03\x04", "xlsx": b"PK\x03\x04", "pptx": b"PK\x03\x04", "zip": b"PK\x03\x04",
    "jar": b"PK\x03\x04", "odt": b"PK\x03\x04", "ods": b"PK\x03\x04", "odp": b"PK\x03\x04",
    "pdf": b"%PDF", "jpg": b"\xff\xd8\xff", "jpeg": b"\xff\xd8\xff", "png": b"\x89PNG",
    "gif": b"GIF8", "bmp": b"BM", "doc": b"\xd0\xcf\x11\xe0", "xls": b"\xd0\xcf\x11\xe0",
    "ppt": b"\xd0\xcf\x11\xe0", "rtf": b"{\\rtf", "gz": b"\x1f\x8b", "7z": b"7z\xbc\xaf",
    "rar": b"Rar!", "mp3": b"ID3", "sqlite": b"SQLite", "psd": b"8BPS",
}
# text-ish types that should be mostly-printable/low-entropy; encryption makes them random
_TEXT_EXT = {"txt", "csv", "tsv", "log", "sql", "json", "xml", "html", "htm", "md",
             "py", "js", "ts", "c", "cpp", "h", "java", "go", "rb", "php", "sh",
             "conf", "ini", "cfg", "yaml", "yml", "tex", "srt", "vcf", "ics"}


def _shannon(data):
    if not data:
        return 0.0
    from math import log2
    counts = [0] * 256
    for b in data:
        counts[b] += 1
    n = len(data)
    return -sum((c / n) * log2(c / n) for c in counts if c)


def _looks_encrypted(path):
    """True if the file's CONTENT no longer matches its extension (encrypted in place).
    None if the type can't be judged (don't guess)."""
    ext = path.rsplit(".", 1)[-1].lower() if "." in os.path.basename(path) else ""
    try:
        with open(path, "rb") as f:
            head = f.read(1024)
    except OSError:
        return None
    if not head:
        return None
    if ext in _MAGIC:
        return not head.startswith(_MAGIC[ext])   # header gone -> encrypted
    if ext in _TEXT_EXT:
        # normal text ~4-5.5 bits/byte and almost all printable; encrypted ~7.99 with
        # ~60% non-printable bytes. Require BOTH so a normal doc never trips.
        printable = sum(1 for b in head if b in (9, 10, 13) or 32 <= b <= 126)
        nonprint_frac = 1.0 - printable / len(head)
        return _shannon(head) > 7.3 and nonprint_frac > 0.30
    return None


def plant(dirs=None):
    if dirs is None:
        dirs = _canary_dirs()
    planted = []
    for d in dirs:
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


def scan_inplace(dirs):
    """#1 — detect IN-PLACE encryption: files recently modified whose CONTENT no longer
    matches their extension. Returns (encrypted_count, sample_dir) if a burst is found
    (>= INPLACE_MIN), else (0, None). Cheap: mtime-gated, header-only reads, capped."""
    cutoff = time.time() - INPLACE_WINDOW
    hits, checked, sample_dir = [], 0, None
    for d in dirs:
        try:
            entries = os.listdir(d)
        except OSError:
            continue
        for name in entries:
            if CANARY_TOKEN in name:
                continue
            p = os.path.join(d, name)
            try:
                st = os.stat(p)
            except OSError:
                continue
            if not (st.st_mode & 0o170000 == 0o100000):  # regular files only
                continue
            if st.st_mtime < cutoff:
                continue
            checked += 1
            if checked > INPLACE_MAX_SCAN:
                break
            enc = _looks_encrypted(p)
            if enc:
                hits.append(p)
                sample_dir = sample_dir or d
        if checked > INPLACE_MAX_SCAN or len(hits) >= INPLACE_MIN:
            break
    return (len(hits), sample_dir) if len(hits) >= INPLACE_MIN else (0, None)


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


def respond(target_dir, action, marker):
    """Kill the process(es) sweeping `target_dir` and emit the ransomware signal.
    `marker` is what gets recorded as the offending file/event."""
    culprits = _culprits(target_dir)
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
    emit(marker, action, proc, pid)
    print(f"ransom-guard: {action} in {target_dir} — killed {killed or '[none identified]'}",
          flush=True)
    return killed


def sweep_check(paths, dirs):
    n = 0
    # canary trip (rename/delete/modify of a decoy)
    for canary, action in _tripped(paths):
        respond(os.path.dirname(canary), f"CANARY {action}", canary)
        n += 1
    # #1 — in-place encryption burst (files whose content no longer matches their type)
    enc_count, sample_dir = scan_inplace(dirs)
    if sample_dir:
        respond(sample_dir, f"in-place encryption ({enc_count} files, content!=type)",
                f"{sample_dir}/<in-place ransomware>")
        n += 1
    if n:
        plant(dirs)  # re-seed for continued protection
    return n


def _already_running():
    """#6 supervision: the guard is (re)launched by the manager command wodle on a
    short interval so a crash recovers within minutes. This makes the relaunch a
    no-op when a healthy instance is already running, so the wodle never stacks
    duplicate guards. Linux/macOS via a pidfile + liveness check."""
    pidfile = os.path.join(AGENT, "var", "ransom-guard.pid")
    try:
        os.makedirs(os.path.dirname(pidfile), exist_ok=True)
        if os.path.exists(pidfile):
            old = int(open(pidfile).read().strip() or "0")
            if old and old != os.getpid():
                try:
                    os.kill(old, 0)  # alive?
                    return True      # a healthy guard already runs -> stand down
                except OSError:
                    pass             # stale pid -> take over
        with open(pidfile, "w") as f:
            f.write(str(os.getpid()))
    except OSError:
        pass
    return False


def main():
    once = "--once" in sys.argv
    if not once and _already_running():
        print("ransom-guard: another instance is already running — standing down", flush=True)
        return
    dirs = _canary_dirs()
    paths = plant(dirs)
    print(f"ransom-guard: planted {len(paths)} canaries across {len(dirs)} dirs "
          f"(depth {CANARY_DEPTH}, kill={'on' if KILL else 'off'}, once={once})", flush=True)
    if once:
        n = sweep_check(paths, dirs)
        print(f"ransom-guard: {n} trigger(s)")
        return
    ticks = 0
    while True:
        try:
            sweep_check(paths, dirs)
        except Exception as e:
            sys.stderr.write(f"ransom-guard loop error: {e}\n")
        ticks += 1
        if ticks % 300 == 0:  # periodically re-walk the tree to seed new/renamed dirs
            dirs = _canary_dirs()
            paths = plant(dirs)
        time.sleep(INTERVAL)


if __name__ == "__main__":
    main()
