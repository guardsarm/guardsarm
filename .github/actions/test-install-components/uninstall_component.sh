#!/bin/bash
package_name=$1
target=$2

if [ -z "$package_name" ] || [ -z "$target" ]; then
    echo "Error: Both package_name and target must be provided."
    echo "Usage: $0 <package_name> <target>"
    exit 1
fi

echo "Uninstalling GuardSarm $target."

if [ -n "$(command -v yum)" ]; then
    uninstall="yum remove -y"
    installed_log="/var/log/yum.log"
elif [ -n "$(command -v dpkg)" ]; then
    uninstall="dpkg --remove"
    installed_log="/var/log/dpkg.log"
else
    echo "Couldn't find type of system"
    exit 1
fi

$uninstall "guardsarm-$target" | tee /packages/status.log

if grep -i " removed.*guardsarm-$target" $installed_log | tee -a /packages/status.log; then
    echo "GuardSarm $target was uninstalled successfully."
    exit 0
else
    echo "Failed to uninstall GuardSarm $target."
    exit 1
fi
