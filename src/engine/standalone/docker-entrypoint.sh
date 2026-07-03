#!/bin/bash

# GuardSarm package builder
# Copyright (C) 2015, Wazuh Inc.
#
# This program is a free software; you can redistribute it
# and/or modify it under the terms of the GNU General Public
# License (version 2) as published by the FSF - Free Software
# Foundation.

set -e

# Main script body

echo "Starting GuardSarm server build process..."

# Set default value for JOBS if not provided
JOBS="${JOBS:-2}"

build_dir="/build_guardsarm"

make -C /workspace/guardsarm/src deps TARGET=manager

if [ "${BUILD_TYPE}" = "debug" ]; then
    make -j${JOBS} -C /workspace/guardsarm/src TARGET=manager DEBUG="yes"
    echo "Debug build completed with ${JOBS} jobs."
else
    make -j${JOBS} -C /workspace/guardsarm/src TARGET=manager
    echo "Release build completed with ${JOBS} jobs."
fi

if [ -d "/opt/gcc-14/lib64" ]; then
    mkdir -p /workspace/guardsarm/gcc-libs
    cp /opt/gcc-14/lib64/libstdc++.so.6 /workspace/guardsarm/gcc-libs/ 2>/dev/null || true
else
    echo "ERROR: /opt/gcc-14/lib64 not found. Cannot copy libstdc++.so.6"
    exit 1
fi
