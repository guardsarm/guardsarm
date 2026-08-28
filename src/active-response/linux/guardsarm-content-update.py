#!/usr/bin/env python3
"""GuardsArm on-agent CONTENT auto-updater (pull model).

Keeps the agent's DATA/CONTENT — YARA rules, active-response scripts, decoders,
CDB lists — current WITHOUT reinstalling the package. Periodically pulls a SIGNED
manifest + changed files from the content channel (packages.guardsarm.com/content/
<channel>/), verifies the manifest's RSA signature against a public key baked into
the agent at install, and applies only the changed files in place (atomic write +
rename). No agent restart: the scanner / AR / rule engine read the files fresh.

Security model (this runs as root/SYSTEM and writes into the agent tree, so it is
hardened against a hostile/compromised content host):
  * The manifest is RSA-SHA256 signed; an invalid/absent signature ABORTS. Only the
    holder of the private key can publish content.
  * Every file's sha256 is pinned IN the signed manifest and re-checked after
    download — a swapped file on the CDN is rejected.
  * Target paths are confined to an ALLOW-LISTED set of subdirs under the agent
    home; any absolute path, "..", or out-of-tree target is refused (no arbitrary
    file write even if the signed manifest is malicious).

Scheduled by a systemd timer / command wodle (hourly). Exit 0 on "nothing to do".
Copyright (C) 2026, GuardsArm.
"""
import hashlib
import json
import os
import platform
import ssl
import subprocess
import sys
import tempfile
import time
import urllib.request

# --------------------------------------------------------------------------- #
def _agent_home():
    if os.environ.get("GS_AGENT_HOME"):
        return os.environ["GS_AGENT_HOME"]
    if (platform.system() == "Darwin") and not os.path.isdir("/var/gsmsec") and os.path.isdir("/Library/Ossec"):
        return "/Library/Ossec"
    return "/var/gsmsec"

HOME = _agent_home()
OS_TAG = "macos" if platform.system() == "Darwin" else "linux"
BASE_URL = os.environ.get("GS_CONTENT_URL", "https://packages.guardsarm.com/content").rstrip("/")
CHANNEL = os.environ.get("GS_CONTENT_CHANNEL", "stable")
# Per-OS content channel: the .py/.sh (linux/macos) and .ps1 (windows) payloads live
# under separate paths so an agent only ever pulls files meant for its platform.
URL = f"{BASE_URL}/{CHANNEL}/{OS_TAG}"
PUBKEY = os.environ.get("GS_CONTENT_PUBKEY", os.path.join(HOME, "etc", "guardsarm-content-update.pub"))
STATE = os.path.join(HOME, "var", "content-update.state")
LOG = os.path.join(HOME, "logs", "content-update.log")
CA = os.environ.get("GS_CONTENT_CA")  # optional pinned CA for the content host
TIMEOUT = 30

# Only these subtrees may be written by a pulled file. A signed-but-malicious
# manifest still cannot drop a file outside them (defence in depth).
ALLOWED_PREFIXES = (
    "etc/yara/", "etc/decoders/", "etc/rules/", "etc/lists/", "etc/shared/",
    "active-response/bin/", "ruleset/", "etc/guardsarm-content-update.pub",
)


def log(msg):
    line = f"{time.strftime('%Y-%m-%dT%H:%M:%SZ', time.gmtime())} {msg}"
    try:
        os.makedirs(os.path.dirname(LOG), exist_ok=True)
        with open(LOG, "a") as f:
            f.write(line + "\n")
    except Exception:
        pass
    print(line, flush=True)


def _get(path, dest):
    ctx = ssl.create_default_context(cafile=CA) if CA else ssl.create_default_context()
    req = urllib.request.Request(f"{URL}/{path}", headers={"User-Agent": "guardsarm-content-update"})
    with urllib.request.urlopen(req, timeout=TIMEOUT, context=ctx) as r, open(dest, "wb") as out:
        while True:
            chunk = r.read(65536)
            if not chunk:
                break
            out.write(chunk)


def _sha256(path):
    h = hashlib.sha256()
    with open(path, "rb") as f:
        for chunk in iter(lambda: f.read(65536), b""):
            h.update(chunk)
    return h.hexdigest()


def _verify_sig(manifest, sig):
    """RSA-SHA256 verify via the bundled openssl (always present with the agent)."""
    if not os.path.isfile(PUBKEY):
        log(f"ERROR: content public key missing ({PUBKEY}) — refusing to apply")
        return False
    try:
        r = subprocess.run(["openssl", "dgst", "-sha256", "-verify", PUBKEY,
                            "-signature", sig, manifest],
                           capture_output=True, text=True, timeout=15)
        return r.returncode == 0 and "Verified OK" in (r.stdout + r.stderr)
    except Exception as e:
        log(f"ERROR: signature verify failed to run: {e}")
        return False


def _safe_target(rel):
    """Resolve rel (from the manifest) to an absolute path INSIDE the agent home,
    confined to ALLOWED_PREFIXES. Returns None if the path is unsafe."""
    rel = rel.replace("\\", "/").lstrip("/")
    if ".." in rel.split("/"):
        return None
    if not any(rel == p or rel.startswith(p) for p in ALLOWED_PREFIXES):
        return None
    home_n = os.path.normpath(HOME)
    dest = os.path.normpath(os.path.join(home_n, rel))
    if dest != home_n and not dest.startswith(home_n + os.sep):
        return None
    return dest


def _install(src, dest, mode):
    os.makedirs(os.path.dirname(dest), exist_ok=True)
    tmp = dest + ".gsupd.tmp"
    with open(src, "rb") as fi, open(tmp, "wb") as fo:
        os.chmod(tmp, int(mode, 8) if mode else 0o640)
        fo.write(fi.read())
        fo.flush()
        os.fsync(fo.fileno())
    os.replace(tmp, dest)  # atomic


def main():
    work = tempfile.mkdtemp(prefix="gscontent.")
    changed = 0
    try:
        mpath, spath = os.path.join(work, "manifest.json"), os.path.join(work, "manifest.json.sig")
        try:
            _get("manifest.json", mpath)
            _get("manifest.json.sig", spath)
        except Exception as e:
            log(f"content channel unreachable ({e}) — will retry next run")
            return 0

        if not _verify_sig(mpath, spath):
            log("ERROR: manifest signature INVALID — aborting (no files applied)")
            return 2

        manifest = json.load(open(mpath))
        newver = str(manifest.get("version", ""))
        curver = ""
        try:
            curver = open(STATE).read().strip()
        except Exception:
            pass
        if newver and newver == curver:
            return 0  # already current
        log(f"content manifest {newver or '?'} (channel {CHANNEL}) — checking {len(manifest.get('files', []))} files")

        for item in manifest.get("files", []):
            rel, want, mode = item.get("path"), item.get("sha256"), str(item.get("mode", "640"))
            if not rel or not want:
                continue
            dest = _safe_target(rel)
            if not dest:
                log(f"REFUSED unsafe path in manifest: {rel}")
                continue
            if os.path.isfile(dest) and _sha256(dest) == want:
                continue  # unchanged
            dl = os.path.join(work, "dl.bin")
            try:
                _get(f"files/{rel}", dl)
            except Exception as e:
                log(f"  download failed for {rel}: {e}")
                continue
            if _sha256(dl) != want:
                log(f"  HASH MISMATCH for {rel} — skipped (manifest says {want[:12]})")
                continue
            _install(dl, dest, mode)
            changed += 1
            log(f"  updated {rel} ({want[:12]}, mode {mode})")

        if newver:
            os.makedirs(os.path.dirname(STATE), exist_ok=True)
            with open(STATE, "w") as f:
                f.write(newver)
        log(f"content update complete: {changed} file(s) changed, now at {newver or '?'}")
        return 0
    finally:
        try:
            import shutil
            shutil.rmtree(work, ignore_errors=True)
        except Exception:
            pass


if __name__ == "__main__":
    sys.exit(main())
