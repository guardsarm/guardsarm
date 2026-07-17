#!/usr/bin/env bash
set -euo pipefail

MANAGER_HOST="${MANAGER_HOST:-host.docker.internal}"
MANAGER_PORT="${MANAGER_PORT:-1514}"
AUTHD_PORT="${AUTHD_PORT:-1515}"
AGENT_NAME="${AGENT_NAME:-$(hostname)}"
AUTHD_PASSWORD="${AUTHD_PASSWORD:-}"

GSMSEC_CONF="/var/gsmsec/etc/gsmsec.conf"
LOG_FILE="/var/gsmsec/logs/gsmsec.log"

echo "[entrypoint] manager=${MANAGER_HOST}:${MANAGER_PORT} authd_port=${AUTHD_PORT} agent_name=${AGENT_NAME}"

if grep -q "<address>" "$GSMSEC_CONF"; then
  sed -i "s|<address>.*</address>|<address>${MANAGER_HOST}</address>|" "$GSMSEC_CONF" || true
else
  echo "[entrypoint] WARN: <address> tag not found in ${GSMSEC_CONF}."
fi

if [[ -n "${AUTHD_PASSWORD}" ]]; then
  /var/gsmsec/bin/guardsarm-agent-auth -A "${AGENT_NAME}" -m "${MANAGER_HOST}" -p "${AUTHD_PORT}" -P "${AUTHD_PASSWORD}" || true
else
  /var/gsmsec/bin/guardsarm-agent-auth -A "${AGENT_NAME}" -m "${MANAGER_HOST}" -p "${AUTHD_PORT}" || true
fi

/var/gsmsec/bin/guardsarm-control start

touch "$LOG_FILE"
tail -F "$LOG_FILE"
