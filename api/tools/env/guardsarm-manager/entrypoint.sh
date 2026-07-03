#!/usr/bin/env bash

set -e

GUARDSARM_CERTS_DIR="/var/guardsarm-manager/etc/certs"
ROLE="$3"
NODE_CERT="${GUARDSARM_CERTS_DIR}/${NODE_NAME}.pem"
NODE_CERT_KEY="${GUARDSARM_CERTS_DIR}/${NODE_NAME}-key.pem"
SERVER_CERT="${GUARDSARM_CERTS_DIR}/manager.pem"
SERVER_CERT_KEY="${GUARDSARM_CERTS_DIR}/manager-key.pem"

echo "Waiting for certificates..."
while [ ! -f "${GUARDSARM_CERTS_DIR}/root-ca.pem" ]; do
  sleep 2
done
echo "Certificates found."

# Set indexer credentials (default: guardsarm-server/guardsarm-server)
echo 'guardsarm-server' | /var/guardsarm-manager/bin/guardsarm-manager-keystore -f indexer -k username
echo 'guardsarm-server' | /var/guardsarm-manager/bin/guardsarm-manager-keystore -f indexer -k password

# Configure guardsarm configuration file and api.yaml based on the Master role
if [ "$ROLE" == "master" ]; then
    python3 /scripts/xml_parser.py /var/guardsarm-manager/etc/guardsarm-manager.conf /scripts/master_guardsarm-manager_conf.xml
    sed -i "s:# access:access:g" /var/guardsarm-manager/api/configuration/api.yaml
    sed -i "s:#  max_request_per_minute\: 300:  max_request_per_minute\: 99999:g" /var/guardsarm-manager/api/configuration/api.yaml
else
    python3 /scripts/xml_parser.py /var/guardsarm-manager/etc/guardsarm-manager.conf /scripts/worker_guardsarm-manager_conf.xml
fi

echo "guardsarm_db.debug=2" >> /var/guardsarm-manager/etc/guardsarm-manager-internal-options.conf
echo "authd.debug=2" >> /var/guardsarm-manager/etc/guardsarm-manager-internal-options.conf
echo "remoted.debug=2" >> /var/guardsarm-manager/etc/guardsarm-manager-internal-options.conf

# Set proper permissions
chmod 500 /var/guardsarm-manager/etc/certs
chmod 400 /var/guardsarm-manager/etc/certs/*
chown -R guardsarm-manager:guardsarm-manager /var/guardsarm-manager/etc/certs

echo "Starting GuardSarm..."
/var/guardsarm-manager/bin/guardsarm-manager-control start

# Keep the container running
while true; do
    sleep 10
done
