#!/bin/sh
#
# Copyright (C) 2015, Wazuh Inc.
# Copyright (C) 2026, GuardSarm, Inc.
# This program is free software; you can redistribute it and/or modify it under the terms of GPLv2
#
# GuardSarm R2 enablement -- install-path symlink helper (INACTIVE by default).
#
# Creates a bidirectional-safe symlink so that a future GuardSarm-native install
# path and the legacy GuardSarm install path resolve to the same directory, allowing
# an operator to trial an R2 path rename without moving data. This script is NOT
# invoked by any installer or package in Release 1.
#
# It is idempotent, reversible, and never deletes data:
#   - If the legacy directory is real and the new path does not exist, it creates
#     new -> legacy (so tools using either path find the same files).
#   - It refuses to act if doing so would overwrite a real directory.
#
# Usage:
#   path-symlink.sh link   <legacy_dir> <new_dir>   # e.g. /var/guardsarm-manager /var/guardsarm-manager
#   path-symlink.sh unlink <new_dir>                 # remove only the symlink we created
#
# Rollback: `path-symlink.sh unlink <new_dir>` removes the symlink; the real
# legacy directory is untouched.

set -eu

action="${1:-}"

case "${action}" in
    link)
        legacy="${2:?legacy dir required}"
        newp="${3:?new dir required}"
        if [ ! -d "${legacy}" ] && [ ! -L "${legacy}" ]; then
            echo "ERROR: legacy directory '${legacy}' does not exist; nothing to link." >&2
            exit 1
        fi
        if [ -L "${newp}" ]; then
            echo "ok: '${newp}' is already a symlink (idempotent no-op)."
            exit 0
        fi
        if [ -e "${newp}" ]; then
            echo "ERROR: '${newp}' already exists and is not a symlink; refusing to overwrite." >&2
            exit 1
        fi
        ln -s "${legacy}" "${newp}"
        echo "linked: ${newp} -> ${legacy}"
        ;;
    unlink)
        newp="${2:?path required}"
        if [ -L "${newp}" ]; then
            rm -f "${newp}"
            echo "unlinked symlink: ${newp} (real data left intact)"
        else
            echo "refusing: '${newp}' is not a symlink; not removing." >&2
            exit 1
        fi
        ;;
    *)
        echo "usage: $0 link <legacy_dir> <new_dir> | unlink <new_dir>" >&2
        exit 2
        ;;
esac
