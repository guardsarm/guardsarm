#!/bin/sh

# Copyright (C) 2015, Wazuh Inc.
# Copyright (C) 2026, GuardSarm, Inc.
# This program is free software; you can redistribute it and/or modify it under the terms of GPLv2
#
# GuardSarm branding layer for the init/control shell scripts.
#
# Sourced by guardsarm-server.sh / guardsarm-client.sh / install.sh to obtain USER-VISIBLE
# branding (product name, vendor). Values come from the repository-root
# branding.json (single source of truth, shared with the C build and Python
# framework). If branding.json is not found, the fallbacks below reproduce the
# original GuardSarm branding, so removing branding.json is a clean rollback.
#
# IMPORTANT: this layer carries ONLY display strings. It defines NO paths, users,
# groups, daemon names, service names or ports -- those stay frozen in the
# scripts that source this file.

# --- Fallbacks (original GuardSarm branding) ---
PRODUCT_NAME="GuardSarm"
PRODUCT_SHORT_NAME="GuardSarm"
PRODUCT_AUTHOR="Wazuh Inc."
PRODUCT_SITE="https://guardsarmsiem.com"
PRODUCT_DISPLAY_VERSION=""

# Minimal JSON string-field extractor (no jq dependency).
# Usage: _branding_get <file> <key>
_branding_get() {
    sed -n "s/.*\"$2\"[[:space:]]*:[[:space:]]*\"\([^\"]*\)\".*/\1/p" "$1" 2>/dev/null | head -n 1
}

# Locate branding.json: explicit env override, then walk up from this script.
_branding_file=""
if [ -n "${GUARDSARM_BRANDING_JSON}" ] && [ -f "${GUARDSARM_BRANDING_JSON}" ]; then
    _branding_file="${GUARDSARM_BRANDING_JSON}"
else
    _branding_dir=$(CDPATH= cd -- "$(dirname -- "$0")" 2>/dev/null && pwd)
    while [ -n "${_branding_dir}" ]; do
        if [ -f "${_branding_dir}/branding.json" ]; then
            _branding_file="${_branding_dir}/branding.json"
            break
        fi
        _parent=$(dirname -- "${_branding_dir}")
        [ "${_parent}" = "${_branding_dir}" ] && break
        _branding_dir="${_parent}"
    done
    # Installed location: alongside the install dir if DIR is set by the caller.
    if [ -z "${_branding_file}" ] && [ -n "${DIR}" ] && [ -f "${DIR}/branding.json" ]; then
        _branding_file="${DIR}/branding.json"
    fi
fi

if [ -n "${_branding_file}" ]; then
    _v=$(_branding_get "${_branding_file}" product_name);        [ -n "${_v}" ] && PRODUCT_NAME="${_v}"
    _v=$(_branding_get "${_branding_file}" product_short_name);  [ -n "${_v}" ] && PRODUCT_SHORT_NAME="${_v}"
    _v=$(_branding_get "${_branding_file}" author);              [ -n "${_v}" ] && PRODUCT_AUTHOR="${_v}"
    _v=$(_branding_get "${_branding_file}" site);                [ -n "${_v}" ] && PRODUCT_SITE="${_v}"
    _v=$(_branding_get "${_branding_file}" product_version);     [ -n "${_v}" ] && PRODUCT_DISPLAY_VERSION="${_v}"
fi

unset _branding_file _branding_dir _parent _v
