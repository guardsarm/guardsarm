#!/bin/bash
set -euo pipefail

# Save current directory
OLD_DIR=$(pwd)
SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)
DEST_DIR="/var/guardsarm-manager/etc/certs"
ORIG_DIR="${SCRIPT_DIR}/certs"
GUARDSARM_USER="guardsarm-manager"
GUARDSARM_GROUP="guardsarm-manager"

# Check if user/group exists
if ! id -u "${GUARDSARM_USER}" >/dev/null 2>&1; then
    echo "User ${GUARDSARM_USER} does not exist. Exiting."
    exit 1
fi

if ! getent group "${GUARDSARM_GROUP}" >/dev/null 2>&1; then
    echo "Group ${GUARDSARM_GROUP} does not exist. Exiting."
    exit 1
fi

# Move to the script directory
cd "${SCRIPT_DIR}"
# Trap to return to the original directory
trap 'cd "$OLD_DIR"' EXIT

# Create destination directory if it doesn't exist
if [ ! -d "${DEST_DIR}" ]; then
    mkdir -p "${DEST_DIR}"
    chown ${GUARDSARM_USER}:${GUARDSARM_GROUP} "${DEST_DIR}"
    chmod 750 "${DEST_DIR}"
fi

echo "Copying certificates to ${DEST_DIR}..."

# Copy certificates
# Array of certificate files to copy
CERT_ORG_FILES=("guardsarm-1-key.pem" "guardsarm-1.pem" "root-ca.pem")
CERT_DST_FILES=("manager-key.pem" "manager.pem" "root-ca.pem")

for i in "${!CERT_ORG_FILES[@]}"; do
    cp "${ORIG_DIR}/${CERT_ORG_FILES[$i]}" "${DEST_DIR}/${CERT_DST_FILES[$i]}"
    chown ${GUARDSARM_USER}:${GUARDSARM_GROUP} ${DEST_DIR}/${CERT_DST_FILES[$i]}
    chmod 640 ${DEST_DIR}/${CERT_DST_FILES[$i]}
    echo "Copied and set permissions for ${CERT_DST_FILES[$i]}"
done

echo "Done copying certificates."
