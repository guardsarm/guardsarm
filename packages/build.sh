#!/bin/bash

# GuardSarm package builder
# Copyright (C) 2015, Wazuh Inc.
# Copyright (C) 2026, GuardSarm.
#
# This program is a free software; you can redistribute it
# and/or modify it under the terms of the GNU General Public
# License (version 2) as published by the FSF - Free Software
# Foundation.
set -e

build_directories() {
  local build_folder=$1
  local guardsarm_dir="$2"
  local future="$3"

  mkdir -p "${build_folder}"
  guardsarm_version=$(awk -F'"' '/"version"[ \t]*:/ {print $4}' guardsarm*/VERSION.json)

  if [[ "$future" == "yes" ]]; then
    guardsarm_version="$(future_version "$build_folder" "$guardsarm_dir" $guardsarm_version)"
    source_dir="${build_folder}/guardsarm-${BUILD_TARGET}-${guardsarm_version}"
  else
    package_name="guardsarm-${BUILD_TARGET}-${guardsarm_version}"
    source_dir="${build_folder}/${package_name}"
    cp -R $guardsarm_dir "$source_dir"
  fi
  echo "$source_dir"
}

# Function to handle future version
future_version() {
  local build_folder="$1"
  local guardsarm_dir="$2"
  local base_version="$3"

  specs_path="$(find $guardsarm_dir/packages -name SPECS|grep $SYSTEM)"

  local major=$(echo "$base_version" | cut -dv -f2 | cut -d. -f1)
  local minor=$(echo "$base_version" | cut -d. -f2)
  local version="${major}.30.0"
  local old_name="guardsarm-${BUILD_TARGET}-${base_version}"
  local new_name=guardsarm-${BUILD_TARGET}-${version}

  local new_guardsarm_dir="${build_folder}/${new_name}"
  cp -R ${guardsarm_dir} "$new_guardsarm_dir"
  find "$new_guardsarm_dir" "${specs_path}" \( -name "*VERSION*" -o -name "*changelog*" \
        -o -name "*.spec" \) -exec sed -i "s/${base_version}/${version}/g" {} \;
  sed -i "s/\$(VERSION)/${major}.${minor}/g" "$new_guardsarm_dir/src/Makefile"
  sed -i "s/${base_version}/${version}/g" $new_guardsarm_dir/src/init/guardsarm-{server,client,local}.sh
  echo "$version"
}

# Function to generate checksum and move files
post_process() {
  local file_path="$1"
  local checksum_flag="$2"
  local source_flag="$3"

  if [[ "$checksum_flag" == "yes" ]]; then
    sha512sum "$file_path" > /var/local/checksum/$(basename "$file_path").sha512
  fi

  if [[ "$source_flag" == "yes" ]]; then
    mv "$file_path" /var/local/guardsarm
  fi
}

# Main script body

# Script parameters
export REVISION="$1"
export JOBS="$2"
debug="$3"
checksum="$4"
future="$5"
src="$6"

build_dir="/build_guardsarm"

source helper_function.sh

if [ -n "${GUARDSARM_VERBOSE}" ]; then
  set -x
fi

# Download source code if it is not shared from the local host
if [ ! -d "/guardsarm-local-src" ] ; then
    curl -sL https://github.com/guardsarm/guardsarm/tarball/${GUARDSARM_BRANCH} | tar zx
    short_commit_hash="$(curl -s https://api.github.com/repos/guardsarm/guardsarm/commits/${GUARDSARM_BRANCH} \
                          | grep '"sha"' | head -n 1| cut -d '"' -f 4 | cut -c 1-7)"
else
      short_commit_hash="$(cd /guardsarm-local-src && git rev-parse --short=7 HEAD)"
fi

# Build directories
source_dir=$(build_directories "$build_dir/${BUILD_TARGET}" "guardsarm*" $future)

guardsarm_version=$(awk -F'"' '/"version"[ \t]*:/ {print $4}' $source_dir/VERSION.json)
# TODO: Improve how we handle package_name
# Changing the "-" to "_" between target and version breaks the convention for RPM or DEB packages.
# For now, I added extra code that fixes it.
package_name="guardsarm-${BUILD_TARGET}-${guardsarm_version}"
specs_path="$(find $source_dir/packages -name SPECS|grep $SYSTEM)"

setup_build "$source_dir" "$specs_path" "$build_dir" "$package_name" "$debug"

set_debug $debug $source_dir

# Installing build dependencies
cd $source_dir
build_deps
build_package $package_name $debug "$short_commit_hash" "$guardsarm_version"

# Post-processing
get_package_and_checksum $guardsarm_version $short_commit_hash $src
