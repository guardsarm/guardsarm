#!/bin/sh
# GuardsArm ransomware RECOVERY provisioning — set up periodic, immutable-ish
# snapshots of user data so guardsarm-restore-snapshot can actually roll back after
# a ransomware incident (isolate -> RESTORE, never pay).  Linux + macOS.
#
# Idempotent: safe to run repeatedly (installer, agent wodle, or by hand).  It
# auto-detects the best snapshot technology present and schedules it:
#
#   Linux root on btrfs  -> read-only btrfs subvolume snapshots on a timer
#   Linux root on zfs    -> zfs snapshots on a timer
#   otherwise (any FS)   -> restic repo (portable single binary) on a timer
#   macOS                -> APFS local snapshots via tmutil on a launchd timer
#
# The scheduled snapshot covers the SAME user-data paths the ransomware FIM/guard
# watch, so what gets encrypted is what gets recovered.  restic is the universal
# fallback because it works on any filesystem and supports local/NAS/S3/SFTP repos.
#
# Env (all optional):
#   GS_SNAPSHOT_MECH      auto|btrfs|zfs|restic|timeshift|tmutil   (default: auto)
#   GS_SNAPSHOT_PATHS     space-separated user-data paths          (default: the FIM set)
#   GS_SNAPSHOT_INTERVAL  systemd OnCalendar / launchd seconds     (default: hourly)
#   GS_SNAPSHOT_REPO      restic repository                        (default: /var/gsmsec/snapshots/restic)
#   GS_SNAPSHOT_PASSFILE  restic password file                     (default: /var/gsmsec/snapshots/.restic-pass)
#   GS_SNAPSHOT_KEEP      restic forget policy  (default: --keep-hourly 24 --keep-daily 7 --keep-weekly 4)
#
# Copyright (C) 2026, GuardsArm.
set -eu

GS_ROOT="${GS_ROOT:-/var/gsmsec}"
[ -d "$GS_ROOT" ] || GS_ROOT="/var/ossec"                      # pre-rebrand install path
MECH="${GS_SNAPSHOT_MECH:-auto}"
PATHS="${GS_SNAPSHOT_PATHS:-/home /root /srv /data /var/www /mnt/shares}"
INTERVAL="${GS_SNAPSHOT_INTERVAL:-hourly}"
REPO="${GS_SNAPSHOT_REPO:-${GS_ROOT}/snapshots/restic}"
PASSFILE="${GS_SNAPSHOT_PASSFILE:-${GS_ROOT}/snapshots/.restic-pass}"
KEEP="${GS_SNAPSHOT_KEEP:---keep-hourly 24 --keep-daily 7 --keep-weekly 4}"
TAG="guardsarm-ransomware-recovery"
LOG="${GS_ROOT}/logs/snapshot-provision.log"

log() { printf '%s %s\n' "$(date '+%Y-%m-%dT%H:%M:%S')" "$*" | tee -a "$LOG" 2>/dev/null || printf '%s\n' "$*"; }
has() { command -v "$1" >/dev/null 2>&1; }

# Only snapshot paths that actually exist on this host.
existing_paths() {
  _p=""
  for d in $PATHS; do [ -d "$d" ] && _p="$_p $d"; done
  printf '%s' "${_p# }"
}

detect() {
  [ "$MECH" != auto ] && { printf '%s' "$MECH"; return; }
  if [ "$(uname -s)" = Darwin ]; then has tmutil && { printf tmutil; return; }; fi
  if has btrfs && btrfs subvolume show / >/dev/null 2>&1; then printf btrfs; return; fi
  if has zfs && zfs list >/dev/null 2>&1; then printf zfs; return; fi
  if has timeshift; then printf timeshift; return; fi
  printf restic                                                  # universal fallback
}

# ---------------------------------------------------------------------------
# restic: portable repo-based snapshots (works on any filesystem)
# ---------------------------------------------------------------------------
ensure_restic() {
  if ! has restic; then
    log "restic not installed; attempting package install"
    if   has apt-get; then apt-get update -qq && apt-get install -y -qq restic
    elif has dnf;     then dnf install -y -q restic
    elif has yum;     then yum install -y -q restic
    elif has zypper;  then zypper -q install -y restic
    elif has apk;     then apk add --no-cache restic
    else log "no package manager for restic; install it manually"; return 1; fi
  fi
  mkdir -p "$(dirname "$REPO")" "$(dirname "$PASSFILE")"
  chmod 700 "$(dirname "$REPO")" 2>/dev/null || true
  if [ ! -f "$PASSFILE" ]; then
    # random repo password; stored root-only. Back this file up off-host — losing
    # it means the repo can't be decrypted.
    (has openssl && openssl rand -base64 32 || head -c 32 /dev/urandom | base64) > "$PASSFILE"
    chmod 600 "$PASSFILE"
    log "generated restic repo password -> $PASSFILE (BACK THIS UP off-host)"
  fi
  export RESTIC_PASSWORD_FILE="$PASSFILE" RESTIC_REPOSITORY="$REPO"
  if ! restic cat config >/dev/null 2>&1; then
    log "initialising restic repo at $REPO"
    restic init >>"$LOG" 2>&1 || { log "restic init failed"; return 1; }
  fi
}

snapshot_now() {
  _mech="$1"; _paths="$(existing_paths)"
  [ -n "$_paths" ] || { log "no user-data paths present to snapshot ($PATHS)"; return 0; }
  case "$_mech" in
    restic)
      export RESTIC_PASSWORD_FILE="$PASSFILE" RESTIC_REPOSITORY="$REPO"
      log "restic backup: $_paths"
      # shellcheck disable=SC2086
      restic backup --tag "$TAG" $_paths >>"$LOG" 2>&1 || { log "restic backup failed"; return 1; }
      # shellcheck disable=SC2086
      restic forget --tag "$TAG" $KEEP --prune >>"$LOG" 2>&1 || true
      ;;
    btrfs)
      _stamp="$(date +%Y%m%d-%H%M%S)"; _dir="/.gs-snapshots"
      mkdir -p "$_dir"
      # snapshot the root subvolume read-only (captures all mounted user data on it)
      btrfs subvolume snapshot -r / "${_dir}/root-${_stamp}" >>"$LOG" 2>&1 \
        && log "btrfs ro-snapshot ${_dir}/root-${_stamp}" || { log "btrfs snapshot failed"; return 1; }
      # retention: keep newest 24
      ls -1dt "${_dir}"/root-* 2>/dev/null | tail -n +25 | while read -r old; do btrfs subvolume delete "$old" >>"$LOG" 2>&1 || true; done
      ;;
    zfs)
      _stamp="$(date +%Y%m%d-%H%M%S)"
      for ds in $(zfs list -H -o name 2>/dev/null); do
        zfs snapshot "${ds}@${TAG}-${_stamp}" >>"$LOG" 2>&1 || true
      done
      log "zfs recursive snapshot @${TAG}-${_stamp}"
      # retention: keep newest 24 per dataset
      for ds in $(zfs list -H -o name 2>/dev/null); do
        zfs list -H -t snapshot -o name -s creation 2>/dev/null | grep "^${ds}@${TAG}-" \
          | head -n -24 | while read -r old; do zfs destroy "$old" >>"$LOG" 2>&1 || true; done
      done
      ;;
    timeshift)
      timeshift --create --comments "$TAG" --tags D >>"$LOG" 2>&1 \
        && log "timeshift snapshot created" || { log "timeshift snapshot failed"; return 1; }
      ;;
    tmutil)
      tmutil localsnapshot >>"$LOG" 2>&1 && log "APFS local snapshot created" || { log "tmutil snapshot failed"; return 1; }
      ;;
    *) log "unknown mechanism '$_mech'"; return 1 ;;
  esac
}

# ---------------------------------------------------------------------------
# Scheduling: systemd timer (Linux) or launchd (macOS)
# ---------------------------------------------------------------------------
SELF="$GS_ROOT/active-response/bin/guardsarm-provision-snapshots"
install_cron_fallback() {   # non-systemd hosts: hourly cron entry
  _line="0 * * * * ${SELF} --run >/dev/null 2>&1"
  if has crontab; then
    ( crontab -l 2>/dev/null | grep -v 'guardsarm-provision-snapshots' ; echo "$_line" ) | crontab - 2>/dev/null \
      && { log "installed hourly cron fallback (no systemd)"; return 0; }
  fi
  if [ -d /etc/cron.d ]; then
    printf '%s\n' "0 * * * * root ${SELF} --run >/dev/null 2>&1" > /etc/cron.d/guardsarm-snapshot 2>/dev/null \
      && { log "installed /etc/cron.d/guardsarm-snapshot (no systemd)"; return 0; }
  fi
  log "no systemd and no cron; the agent's daily provision wodle is the only cadence"
}

install_schedule_systemd() {
  has systemctl || { install_cron_fallback; return 0; }
  cat > /etc/systemd/system/guardsarm-snapshot.service <<EOF
[Unit]
Description=GuardsArm ransomware-recovery snapshot
After=local-fs.target
[Service]
Type=oneshot
Environment=GS_SNAPSHOT_MECH=${MECH}
Environment=GS_SNAPSHOT_PATHS=${PATHS}
Environment=GS_SNAPSHOT_REPO=${REPO}
Environment=GS_SNAPSHOT_PASSFILE=${PASSFILE}
ExecStart=${SELF} --run
Nice=10
IOSchedulingClass=idle
EOF
  cat > /etc/systemd/system/guardsarm-snapshot.timer <<EOF
[Unit]
Description=GuardsArm ransomware-recovery snapshot timer
[Timer]
OnCalendar=${INTERVAL}
Persistent=true
RandomizedDelaySec=300
[Install]
WantedBy=timers.target
EOF
  systemctl daemon-reload
  systemctl enable --now guardsarm-snapshot.timer >>"$LOG" 2>&1 || true
  log "installed + enabled guardsarm-snapshot.timer (OnCalendar=${INTERVAL})"
}

install_schedule_launchd() {
  _plist=/Library/LaunchDaemons/com.guardsarm.snapshot.plist
  _secs=3600; [ "$INTERVAL" = hourly ] || _secs="$INTERVAL"
  cat > "$_plist" <<EOF
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0"><dict>
  <key>Label</key><string>com.guardsarm.snapshot</string>
  <key>ProgramArguments</key><array><string>${SELF}</string><string>--run</string></array>
  <key>StartInterval</key><integer>${_secs}</integer>
  <key>RunAtLoad</key><true/>
</dict></plist>
EOF
  launchctl unload "$_plist" 2>/dev/null || true
  launchctl load -w "$_plist" 2>/dev/null || true
  log "installed launchd com.guardsarm.snapshot (every ${_secs}s)"
}

main() {
  mkdir -p "$(dirname "$LOG")" 2>/dev/null || true
  _mech="$(detect)"
  log "provisioning ransomware-recovery snapshots: mechanism=${_mech}"

  case "$1" in
    --run)                                   # scheduled invocation: just take a snapshot
      [ "$_mech" = restic ] && ensure_restic
      snapshot_now "$_mech"
      return $?
      ;;
    --provision|"")                          # set up mechanism + schedule + first snapshot
      [ "$_mech" = restic ] && { ensure_restic || { log "restic provisioning failed"; return 1; }; }
      MECH="$_mech"                          # pin the resolved mechanism into the schedule
      if [ "$(uname -s)" = Darwin ]; then install_schedule_launchd; else install_schedule_systemd; fi
      snapshot_now "$_mech" || log "initial snapshot failed (schedule still installed)"
      log "provisioning complete; restore with: ${GS_ROOT}/active-response/bin/restore-snapshot --list"
      ;;
    --status)
      "${GS_ROOT}/active-response/bin/restore-snapshot" --list 2>/dev/null || echo "restore-snapshot not installed"
      ;;
    *) echo "usage: $0 [--provision|--run|--status]"; return 2 ;;
  esac
}

main "${1:-}"
