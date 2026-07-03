#!/bin/bash
set -e

# Set correct ownership and permissions for certificates in /etc/guardsarm-dashboard/certs/
echo "Setting up certificate permissions..."
mkdir -p /etc/guardsarm-dashboard/certs
cp /certs/root-ca.pem /etc/guardsarm-dashboard/certs/root-ca.pem
cp /certs/dashboard.pem /etc/guardsarm-dashboard/certs/dashboard.pem
cp /certs/dashboard-key.pem /etc/guardsarm-dashboard/certs/dashboard-key.pem
chown -R guardsarm-dashboard:guardsarm-dashboard /etc/guardsarm-dashboard/certs
chmod 640 /etc/guardsarm-dashboard/certs/*
chmod 750 /etc/guardsarm-dashboard/certs/

# Start guardsarm-dashboard service
# sudo -u guardsarm-dashboard /usr/share/guardsarm-dashboard/bin/opensearch-dashboards -c /etc/guardsarm-dashboard/opensearch_dashboards.yml
echo "Starting guardsarm-dashboard..."

sudo -u guardsarm-dashboard /usr/share/guardsarm-dashboard/bin/opensearch-dashboards -c /etc/guardsarm-dashboard/opensearch_dashboards.yml
