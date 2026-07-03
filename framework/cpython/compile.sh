#!/bin/bash
# Copyright (C) 2015, Wazuh Inc.
#
# This program is a free software; you can redistribute it
# and/or modify it under the terms of the GNU General Public
# License (version 2) as published by the FSF - Free Software
# Foundation.

set -euo pipefail

GUARDSARM_HOST_DIR=/guardsarm_host
GUARDSARM_ROOT_DIR=/guardsarm #Important: Do not change this path
GUARDSARM_INSTALLDIR=/var/guardsarm-manager
CPYTHON_DIR=$GUARDSARM_ROOT_DIR/src/external/cpython
OUTPUT_DIR=/output

main() {
    # Parse script arguments
    parse_args "$@" || exit 1
    # Get guardsarm repository
    get_guardsarm_repo
    # Download guardsarm precompiled dependencies
    make -C "$GUARDSARM_ROOT_DIR/src" PYTHON_SOURCE=y deps -j

    PYTHON_VERSION=$(cat $GUARDSARM_ROOT_DIR/framework/.python-version)

    if $BUILD_CPYTHON; then
        # Build CPython from sources
        rm -rf "$CPYTHON_DIR"
        download_cpython
        customize_cpython
        build_cpython
    fi

    if $BUILD_DEPS || $BUILD_CPYTHON; then
        download_wheels
    fi

    mimic_full_guardsarm_installation
    generate_artifacts
}

get_guardsarm_repo() {
    if [ -z "${GUARDSARM_BRANCH:-}" ]; then
        cp -rf $GUARDSARM_HOST_DIR $GUARDSARM_ROOT_DIR
        # Clean previous builds
        find "$GUARDSARM_ROOT_DIR/src/external" -mindepth 1 ! -name 'CMakeLists.txt' -exec rm -rf {} +
        make clean -j -C "$GUARDSARM_ROOT_DIR/src"
    else
        git clone --branch "$GUARDSARM_BRANCH" --depth 1 https://github.com/guardsarm/guardsarm.git  "$GUARDSARM_ROOT_DIR"
    fi
}

download_cpython() {
    git clone --branch "v$PYTHON_VERSION" --depth 1 https://github.com/python/cpython.git "$CPYTHON_DIR"
}

customize_cpython() {
    cp -f $GUARDSARM_ROOT_DIR/framework/cpython/custom/Setup.local $CPYTHON_DIR/Modules
    cp -f $GUARDSARM_ROOT_DIR/framework/cpython/custom/Setup.stdlib.in $CPYTHON_DIR/Modules
}

build_cpython() {
    make -j -C "$GUARDSARM_ROOT_DIR/src" build_python TARGET=manager INSTALLDIR=$GUARDSARM_INSTALLDIR OPTIMIZE_CPYTHON=yes
}

mimic_full_guardsarm_installation() {
    # Force build of libguardsarmext
    make -j -C "$GUARDSARM_ROOT_DIR/src" build-external TARGET=manager INSTALLDIR=$GUARDSARM_INSTALLDIR
    # Install only libguardsarmext to avoid full server compilation & installation
    mkdir -p "$GUARDSARM_INSTALLDIR/lib"
    install -m 0750 $GUARDSARM_ROOT_DIR/src/build/lib/libguardsarmext.so "$GUARDSARM_INSTALLDIR/lib"
    # Install python interpreter and its dependencies
    make -j -C "$GUARDSARM_ROOT_DIR/src" install_dependencies INSTALLDIR=$GUARDSARM_INSTALLDIR
}

generate_artifacts() {
    # Compress built cpython
    cd $GUARDSARM_ROOT_DIR/src/external && tar -zcf "$OUTPUT_DIR/cpython_$ARCH.tar.gz" --owner=0 --group=0 cpython
    # Compress ready-to-use CPython
    cd $GUARDSARM_INSTALLDIR/framework/python && tar -zcf "$OUTPUT_DIR/cpython.tar.gz" --owner=0 --group=0 .
}

download_wheels() {
    # Install Python3 to download wheels
    yum install python3 -y
    python3 -m pip install --upgrade pip
    # Remove existing dependencies
    rm -rf "$CPYTHON_DIR/Dependencies"
    # Create dependencies directory
    mkdir -p "$CPYTHON_DIR/Dependencies"
    # Download wheels
    python3 -m pip download --requirement "$GUARDSARM_ROOT_DIR/framework/requirements.txt"  --no-deps --dest "$CPYTHON_DIR/Dependencies"  --python-version "$PYTHON_VERSION" --no-cache-dir
    # Create index
    python3 -m pip install piprepo && piprepo build "$CPYTHON_DIR/Dependencies"
}

parse_args() {
    BUILD_CPYTHON=false
    BUILD_DEPS=false

    while [[ $# -gt 0 ]]; do
        case "$1" in
            --build-cpython)
                BUILD_CPYTHON=true
                ;;
            --build-deps)
                BUILD_DEPS=true
                ;;
            --guardsarm-branch)
                GUARDSARM_BRANCH="$2"
                ;;
            *)
                echo "ERROR: Unrecognized parameter: $1" >&2
                return 1
                ;;
        esac
        shift
    done
}

main "$@"
