#!/bin/env bash

# Check if GUARDSARM_HOME is set
if [ -z "$GUARDSARM_HOME" ]; then
    GUARDSARM_HOME="/var/guardsarm-manager"
fi

if [ ! -d "$GUARDSARM_HOME" ]; then
    echo "GuardSarm home directory $GUARDSARM_HOME does not exist."
    exit 1
fi

# Mount proc filesystem if it does not exist (Mounted for development purposes)
if ! mountpoint -q ${GUARDSARM_HOME}/proc; then
    mkdir -p ${GUARDSARM_HOME}/proc
    mount -t proc proc ${GUARDSARM_HOME}/proc
    if [ $? -ne 0 ]; then
        echo "Failed to mount proc filesystem at ${GUARDSARM_HOME}/proc"
        exit 1
    fi
    echo "Mounted proc filesystem at ${GUARDSARM_HOME}/proc"
else
    echo "Proc filesystem is already mounted at ${GUARDSARM_HOME}/proc"
fi
