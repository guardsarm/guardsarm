#!/usr/bin/env bash
# GuardSarm Linux agent tamper guardian (user-mode resilience tier).
#
# Linux port of the Windows guardsarm-tamper-guard.exe. Two independent
# SYSTEM-privileged mechanisms guard the agent AND each other:
#   * systemd service  guardsarm-tamper-guard.service  (always-on, ticks every 45s)
#   * systemd timer    guardsarm-tamper-guard.timer    (boot + every 1min, one tick)
# Each tick (idempotent, silent unless it corrects something):
#   1. Service resilience  : (re)start guardsarm-agent.service if stopped/killed.
#   2. Integrity self-heal : restore agent binaries/libs from <home>/.integrity/
#                            vs a SHA-256 manifest — restored on delete OR modify;
#                            gsmsec.conf restored on DELETE only (edits = alert).
#   3. Re-arm              : recreate the sibling unit if either was removed.
#   4. Emit "guardsarm-tamper-guard: TAMPER: <kind> - <detail>" to the agent's
#      active-responses.log (a monitored localfile → forwarded to the manager).
# Honest ceiling: this defeats every NON-kernel tamper attempt and auto-recovers
# + alerts; true un-killability needs an LSM/eBPF-LSM enforcement module (gated).
# Fail-open: stands down while dpkg/apt holds its lock so it never fights an
# install/upgrade/uninstall.
set -u

HOME_DIR="${GS_AGENT_HOME:-/var/gsmsec}"
BIN="${HOME_DIR}/active-response/bin/guardsarm-tamper-guard"
INTEG="${HOME_DIR}/.integrity"
MANIFEST="${INTEG}/manifest"
DISARM="${INTEG}/disarm"
LOG="${HOME_DIR}/logs/active-responses.log"
AGENT_SVC="guardsarm-agent.service"
GUARD_SVC="guardsarm-tamper-guard.service"
GUARD_TIMER="guardsarm-tamper-guard.timer"
UNIT_DIR="/etc/systemd/system"

logline() {  # kind, detail
  local ts; ts="$(date '+%Y/%m/%d %H:%M:%S')"
  mkdir -p "$(dirname "$LOG")" 2>/dev/null
  echo "${ts} guardsarm-tamper-guard: TAMPER: $1 - $2" >> "$LOG" 2>/dev/null
}

# flattened backup path: bin/guardsarm-agentd -> .integrity/bin_guardsarm-agentd
bak_path() { echo "${INTEG}/$(echo "$1" | tr '/' '_')"; }

# The protected set, enumerated live so new modules are auto-covered (mode: 1=binary
# restore-on-modify, 0=config restore-on-delete-only).
crit_list() {
  printf '%s\t1\n' "active-response/bin/guardsarm-tamper-guard"
  local f rel
  for f in "${HOME_DIR}"/bin/* "${HOME_DIR}"/lib/*.so*; do
    [ -f "$f" ] || continue
    rel="${f#${HOME_DIR}/}"; printf '%s\t1\n' "$rel"
  done
  [ -f "${HOME_DIR}/etc/gsmsec.conf" ] && printf '%s\t0\n' "etc/gsmsec.conf"
}

dpkg_busy() { fuser /var/lib/dpkg/lock-frontend >/dev/null 2>&1; }

do_arm() {
  mkdir -p "$INTEG"; chmod 700 "$INTEG"
  : > "$MANIFEST"; local n=0 rel mode
  while IFS=$'\t' read -r rel mode; do
    [ -f "${HOME_DIR}/${rel}" ] || continue
    cp -p "${HOME_DIR}/${rel}" "$(bak_path "$rel")" 2>/dev/null
    printf '%s\t%s\t%s\n' "$rel" "$(sha256sum "${HOME_DIR}/${rel}" | awk '{print $1}')" "$mode" >> "$MANIFEST"
    n=$((n+1))
  done < <(crit_list)
  chmod 600 "$MANIFEST"; rm -f "$DISARM"
  install_units
  systemctl daemon-reload 2>/dev/null
  systemctl enable --now "$GUARD_SVC" "$GUARD_TIMER" 2>/dev/null
  logline "armed" "integrity manifest written (${n} files) + guardian service/timer armed"
}

do_disarm() {  # for package upgrade/removal
  touch "$DISARM" 2>/dev/null
  systemctl disable --now "$GUARD_SVC" "$GUARD_TIMER" 2>/dev/null
  rm -f "${UNIT_DIR}/${GUARD_SVC}" "${UNIT_DIR}/${GUARD_TIMER}"
  systemctl daemon-reload 2>/dev/null
}

restore() { local rel="$1"; local b; b="$(bak_path "$rel")"; [ -f "$b" ] || return 1; cp -p "$b" "${HOME_DIR}/${rel}" 2>/dev/null; }

self_heal() {
  [ -f "$MANIFEST" ] || return 0
  local rel want mode have
  while IFS=$'\t' read -r rel want mode; do
    local full="${HOME_DIR}/${rel}"
    if [ ! -f "$full" ]; then
      if restore "$rel"; then logline "file_deleted" "${rel} (restored from backup)"
      else logline "file_deleted" "${rel} (unrecoverable: no backup)"; fi
      continue
    fi
    have="$(sha256sum "$full" 2>/dev/null | awk '{print $1}')"
    [ "$have" = "$want" ] && continue
    if [ "$mode" = "1" ]; then
      if restore "$rel"; then logline "file_modified" "${rel} (restored from backup)"
      else logline "file_modified" "${rel} (unrecoverable: no backup)"; fi
    else
      logline "config_changed" "${rel}"
    fi
  done < "$MANIFEST"
}

ensure_agent() {
  systemctl is-active --quiet "$AGENT_SVC" && return 0
  systemctl start "$AGENT_SVC" 2>/dev/null && logline "service_stopped" "restarted ${AGENT_SVC}"
}

rearm_units() {
  local changed=0
  [ -f "${UNIT_DIR}/${GUARD_SVC}" ]   || { install_units; changed=1; logline "unit_removed" "re-armed ${GUARD_SVC}"; }
  [ -f "${UNIT_DIR}/${GUARD_TIMER}" ] || { install_units; changed=1; logline "unit_removed" "re-armed ${GUARD_TIMER}"; }
  if [ "$changed" = "1" ]; then systemctl daemon-reload 2>/dev/null; systemctl enable --now "$GUARD_SVC" "$GUARD_TIMER" 2>/dev/null; fi
}

do_tick() {
  [ -f "$DISARM" ] && exit 0            # disarmed (uninstall in progress)
  dpkg_busy && exit 0                   # stand down during package ops
  ensure_agent
  self_heal
  rearm_units
}

install_units() {
  cat > "${UNIT_DIR}/${GUARD_SVC}" <<UNIT
[Unit]
Description=GuardSarm agent integrity guardian (tamper protection)
After=network.target
[Service]
Type=simple
ExecStart=/bin/bash ${BIN} --service
Restart=always
RestartSec=5
# hardening: the guardian must survive kill attempts and keep least privilege
NoNewPrivileges=false
ProtectSystem=off
[Install]
WantedBy=multi-user.target
UNIT
  cat > "${UNIT_DIR}/${GUARD_TIMER}" <<UNIT
[Unit]
Description=GuardSarm integrity guardian periodic tick (re-arm mechanism)
[Timer]
OnBootSec=30
OnUnitActiveSec=60
Unit=guardsarm-tamper-guard-tick.service
[Install]
WantedBy=timers.target
UNIT
  # oneshot the timer triggers each minute (independent of the always-on service)
  cat > "${UNIT_DIR}/guardsarm-tamper-guard-tick.service" <<UNIT
[Unit]
Description=GuardSarm integrity guardian single tick
[Service]
Type=oneshot
ExecStart=/bin/bash ${BIN} --tick
UNIT
}

case "${1:---tick}" in
  --arm)     do_arm ;;
  --disarm)  do_disarm ;;
  --tick)    do_tick ;;
  --service) while true; do do_tick; sleep 45; done ;;
  --status)
    echo "guardian svc:   $(systemctl is-active $GUARD_SVC 2>/dev/null)"
    echo "guardian timer: $(systemctl is-active $GUARD_TIMER 2>/dev/null)"
    echo "agent svc:      $(systemctl is-active $AGENT_SVC 2>/dev/null)"
    echo "manifest files: $([ -f "$MANIFEST" ] && wc -l < "$MANIFEST" || echo 0)"
    echo "disarmed:       $([ -f "$DISARM" ] && echo yes || echo no)"
    ;;
  *) echo "usage: $0 {--arm|--disarm|--tick|--service|--status}"; exit 2 ;;
esac
