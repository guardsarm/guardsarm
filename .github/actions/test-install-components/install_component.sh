#!/bin/bash
package_name=$1
target=$2

# Check parameters
if [ -z "$package_name" ] || [ -z "$target" ]; then
    echo "Error: Both package_name and target must be provided."
    echo "Usage: $0 <package_name> <target>"
    exit 1
fi

echo "Installing GuardSarm $target."

if [ -n "$(command -v yum)" ]; then
    install="yum --setopt=retries=5 --setopt=timeout=30 install -y --nogpgcheck"
    installed_log="/var/log/yum.log"
elif [ -n "$(command -v dpkg)" ]; then
    install="dpkg --install"
    installed_log="/var/log/dpkg.log"
else
    common_logger -e "Couldn't find type of system"
    exit 1
fi

GUARDSARM_MANAGER="10.0.0.2" $install "/packages/$package_name"| tee /packages/status.log
grep -i " installed.*guardsarm-$target" $installed_log| tee -a /packages/status.log

# Retrieve guardsarm gid and uid
if [ "$target" = "manager" ]; then
    guardsarm_gid=$(getent group guardsarm-manager | cut -d: -f3)
    guardsarm_uid=$(getent passwd guardsarm-manager | cut -d: -f3)
else
    guardsarm_gid=$(getent group guardsarm | cut -d: -f3)
    guardsarm_uid=$(getent passwd guardsarm | cut -d: -f3)
fi

echo $guardsarm_gid > /tests/guardsarm_gid
echo $guardsarm_uid > /tests/guardsarm_uid
