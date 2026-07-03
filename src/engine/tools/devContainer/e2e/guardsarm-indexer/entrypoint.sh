#!/bin/bash
set -e

# Wait for certificates to be mounted
echo "Checking for certificates..."

# Set correct ownership and permissions for certificates in /etc/guardsarm-indexer/certs/
if [ -d "/etc/guardsarm-indexer/certs" ]; then
    echo "Setting up certificate permissions..."
    cp /certs/node-1-key.pem /etc/guardsarm-indexer/certs/indexer-key.pem
    cp /certs/node-1.pem /etc/guardsarm-indexer/certs/indexer.pem
    cp /certs/root-ca.pem /etc/guardsarm-indexer/certs/root-ca.pem
    cp /certs/admin.pem /etc/guardsarm-indexer/certs/admin.pem
    cp /certs/admin-key.pem /etc/guardsarm-indexer/certs/admin-key.pem
    chown -R guardsarm-indexer:guardsarm-indexer /etc/guardsarm-indexer/certs/
    chmod 640 /etc/guardsarm-indexer/certs/*
fi

# Start guardsarm-indexer service
echo "Starting guardsarm-indexer..."
service guardsarm-indexer start

# Wait for service to be ready
echo "Waiting for guardsarm-indexer to be ready..."
sleep 3

# Check if server is up 'service guardsarm-indexer status'
service guardsarm-indexer status

if [ $? -ne 0 ]; then
    echo "GuardSarm-indexer service failed to start."
    service guardsarm-indexer restart
    sleep 3
    service guardsarm-indexer status
    if [ $? -ne 0 ]; then
        echo "GuardSarm-indexer service failed to start after restart. Exiting."
        exit 1
    fi
fi


# Initialize security only if not already done
INIT_FLAG="/etc/guardsarm-indexer-init/.security_initialized"
if [ ! -f "$INIT_FLAG" ]; then
    echo "Initializing indexer security for the first time..."
    sleep 20
    /usr/share/guardsarm-indexer/bin/indexer-security-init.sh

    # Create flag to indicate security has been initialized
    mkdir -p /etc/guardsarm-indexer-init
    touch "$INIT_FLAG"
    echo "Security initialization completed and flag created."
else
    echo "Security already initialized, skipping initialization."
fi




echo "GuardSarm-indexer is ready!"


# Keep container running - if CMD was provided, execute it, otherwise keep alive
if [ "$#" -gt 0 ] && [ "$1" != "/bin/bash" ]; then
    exec "$@"
else
    tail -f /dev/null
fi
