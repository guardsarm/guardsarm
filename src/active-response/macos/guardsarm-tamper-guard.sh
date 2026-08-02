#!/bin/bash
# GuardSarm macOS agent tamper guardian (user-mode resilience tier).
#
# macOS/launchd port of the Windows/Linux guardsarm-tamper-guard. Two independent
# root LaunchDaemons guard the agent AND each other:
#   * com.guardsarm.tamperguard        (RunAtLoad + KeepAlive, ticks every 45s)
#   * com.guardsarm.tamperguard.timer  (StartInterval 60, one tick — re-arm path)
# Each tick (idempotent, silent unless it corrects something):
#   1. Service resilience  : bootstrap com.guardsarm.agent if it is not loaded.
#   2. Integrity self-heal : restore agent binaries/libs from <DIR>/.integrity/ vs a
#                            SHA-256 manifest (restore on delete OR modify); the
#                            config is restored on DELETE only (edits = alert).
#   3. Re-arm              : reload the sibling LaunchDaemon if its plist was removed.
#   4. Emit "guardsarm-tamper-guard: TAMPER: <kind> - <detail>" to the agent's
#      active-responses.log (a monitored localfile → forwarded to the manager; the
#      manager decoder matches guardsarm-tamper-guard.*TAMPER: for all OSes).
# Fail-open: stands down while a disarm marker is present (installer/uninstall).
#
# NOTE: written from source for parity with Linux/Windows but NOT yet built/tested
# on a real Mac (no macOS build host available) — verify on macOS before shipping.
set -u

DIR="${GS_AGENT_HOME:-/Library/gsmsec}"
BIN="${DIR}/active-response/bin/guardsarm-tamper-guard"
INTEG="${DIR}/.integrity"
MANIFEST="${INTEG}/manifest"
DISARM="${INTEG}/disarm"
LOG="${DIR}/logs/active-responses.log"
AGENT_LABEL="com.guardsarm.agent"
GUARD_LABEL="com.guardsarm.tamperguard"
TIMER_LABEL="com.guardsarm.tamperguard.timer"
LD="/Library/LaunchDaemons"

logline() {  # kind, detail
  local ts; ts="$(date '+%Y/%m/%d %H:%M:%S')"
  mkdir -p "$(dirname "$LOG")" 2>/dev/null
  echo "${ts} guardsarm-tamper-guard: TAMPER: $1 - $2" >> "$LOG" 2>/dev/null
}
sha() { shasum -a 256 "$1" 2>/dev/null | awk '{print $1}'; }
bak_path() { echo "${INTEG}/$(echo "$1" | tr '/' '_')"; }

crit_list() {
  printf '%s\t1\n' "active-response/bin/guardsarm-tamper-guard"
  local f rel
  for f in "${DIR}"/bin/* "${DIR}"/lib/*.dylib "${DIR}"/lib/*.so; do
    [ -f "$f" ] || continue; rel="${f#${DIR}/}"; printf '%s\t1\n' "$rel"
  done
  [ -f "${DIR}/etc/gsmsec.conf" ] && printf '%s\t0\n' "etc/gsmsec.conf"
}

write_plist() {  # label, "program-args-xml", extra-keys-xml
  cat > "${LD}/$1.plist" <<PLIST
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0"><dict>
  <key>Label</key><string>$1</string>
  <key>ProgramArguments</key><array>$2</array>
  <key>RunAtLoad</key><true/>
$3
</dict></plist>
PLIST
  chown root:wheel "${LD}/$1.plist" 2>/dev/null; chmod 644 "${LD}/$1.plist" 2>/dev/null
}

install_units() {
  write_plist "$GUARD_LABEL" \
    "<string>/bin/bash</string><string>${BIN}</string><string>--service</string>" \
    "  <key>KeepAlive</key><true/>"
  write_plist "$TIMER_LABEL" \
    "<string>/bin/bash</string><string>${BIN}</string><string>--tick</string>" \
    "  <key>StartInterval</key><integer>60</integer>"
}

load_units()   { launchctl bootstrap system "${LD}/${GUARD_LABEL}.plist" 2>/dev/null; launchctl bootstrap system "${LD}/${TIMER_LABEL}.plist" 2>/dev/null; }
unload_units() { launchctl bootout system "${LD}/${GUARD_LABEL}.plist" 2>/dev/null; launchctl bootout system "${LD}/${TIMER_LABEL}.plist" 2>/dev/null; }

do_arm() {
  mkdir -p "$INTEG"; chmod 700 "$INTEG"; : > "$MANIFEST"; local n=0 rel mode
  while IFS=$'\t' read -r rel mode; do
    [ -f "${DIR}/${rel}" ] || continue
    cp -p "${DIR}/${rel}" "$(bak_path "$rel")" 2>/dev/null
    printf '%s\t%s\t%s\n' "$rel" "$(sha "${DIR}/${rel}")" "$mode" >> "$MANIFEST"; n=$((n+1))
  done < <(crit_list)
  chmod 600 "$MANIFEST"; rm -f "$DISARM"
  install_units; load_units
  logline "armed" "integrity manifest written (${n} files) + guardian LaunchDaemons armed"
}
do_disarm() { touch "$DISARM" 2>/dev/null; unload_units; rm -f "${LD}/${GUARD_LABEL}.plist" "${LD}/${TIMER_LABEL}.plist"; }

restore() { local b; b="$(bak_path "$1")"; [ -f "$b" ] || return 1; cp -p "$b" "${DIR}/$1" 2>/dev/null; }
self_heal() {
  [ -f "$MANIFEST" ] || return 0; local rel want mode have full
  while IFS=$'\t' read -r rel want mode; do
    full="${DIR}/${rel}"
    if [ ! -f "$full" ]; then
      if restore "$rel"; then logline "file_deleted" "${rel} (restored from backup)"; else logline "file_deleted" "${rel} (unrecoverable: no backup)"; fi; continue
    fi
    have="$(sha "$full")"; [ "$have" = "$want" ] && continue
    if [ "$mode" = "1" ]; then
      if restore "$rel"; then logline "file_modified" "${rel} (restored from backup)"; else logline "file_modified" "${rel} (unrecoverable: no backup)"; fi
    else logline "config_changed" "${rel}"; fi
  done < "$MANIFEST"
}
ensure_agent() {
  launchctl print "system/${AGENT_LABEL}" >/dev/null 2>&1 && return 0
  [ -f "${LD}/${AGENT_LABEL}.plist" ] && launchctl bootstrap system "${LD}/${AGENT_LABEL}.plist" 2>/dev/null && logline "service_stopped" "reloaded ${AGENT_LABEL}"
}
rearm_units() {
  [ -f "${LD}/${GUARD_LABEL}.plist" ] && [ -f "${LD}/${TIMER_LABEL}.plist" ] && return 0
  install_units; load_units; logline "unit_removed" "re-armed guardian LaunchDaemons"
}
do_tick() { [ -f "$DISARM" ] && exit 0; ensure_agent; self_heal; rearm_units; }

case "${1:---tick}" in
  --arm)     do_arm ;;
  --disarm)  do_disarm ;;
  --tick)    do_tick ;;
  --service) while true; do do_tick; sleep 45; done ;;
  --status)
    echo "guardian: $(launchctl print system/${GUARD_LABEL} >/dev/null 2>&1 && echo loaded || echo not-loaded)"
    echo "agent:    $(launchctl print system/${AGENT_LABEL} >/dev/null 2>&1 && echo loaded || echo not-loaded)"
    echo "manifest: $([ -f "$MANIFEST" ] && wc -l < "$MANIFEST" | tr -d ' ' || echo 0) files"
    echo "disarmed: $([ -f "$DISARM" ] && echo yes || echo no)" ;;
  *) echo "usage: $0 {--arm|--disarm|--tick|--service|--status}"; exit 2 ;;
esac
