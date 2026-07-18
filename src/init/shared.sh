#!/bin/sh

# Copyright (C) 2026 GuardSarm, Inc.
# Shared variables and functions
# Author: Daniel B. Cid <daniel.cid@gmail.com>

### Setting up variables
# VERSION.json lives at the source/install root, but shared.sh is sourced from
# varying CWDs -- the source build runs from src/.. (VERSION.json present) while a
# .deb/.rpm postinst runs with CWD=/ (dpkg), where a bare "VERSION.json" is not
# found. That spammed "awk: cannot open VERSION.json" during install and left
# VERSION/REVISION empty. Resolve it from a few known locations instead of the CWD.
_GS_VERSION_JSON=""
for _cand in "VERSION.json" "./VERSION.json" "../VERSION.json" \
             "/var/gsmsec/VERSION.json" "/var/guardsarm-manager/VERSION.json"; do
    if [ -f "${_cand}" ]; then _GS_VERSION_JSON="${_cand}"; break; fi
done
if [ -n "${_GS_VERSION_JSON}" ]; then
    VERSION="v$(awk -F'"' '/"version"[ \t]*:/ {print $4}' "${_GS_VERSION_JSON}")"
    REVISION=$(awk -F'"' '/"stage"[ \t]*:/ {print $4}' "${_GS_VERSION_JSON}")
else
    VERSION=""
    REVISION=""
fi
UNAME=`uname -snr`
NUNAME=`uname`
VUNAME=`uname -r`

# If whoami does not exist, try id
if command -v whoami > /dev/null 2>&1 ; then
    ME=`whoami`
else
    ME=`id | cut -d " " -f 1`
    if [ "X${ME}" = "Xuid=0(root)" ]; then
        ME="root"
    fi
fi

# If hostname does not exist, try 'uname -n'
if command -v hostname > /dev/null 2>&1 ; then
    HOST=`hostname`
else
    HOST=`uname -n`
fi

NAMESERVERS=`cat /etc/resolv.conf | grep "^nameserver" | cut -d " " -sf 2`
NAMESERVERS2=`cat /etc/resolv.conf | grep "^nameserver" | cut -sf 2`
HOST_CMD=`command -v host 2>/dev/null`
# GuardSarm display name for the installer banner/prompts (see branding.json).
# Display only -- INSTALLDIR, GUARDSARM_CONF, users/groups etc. below stay frozen.
NAME="GuardSarm"
INSTYPE="manager"
# Default installation directory
INSTALLDIR="/var/guardsarm-manager";
PREINSTALLEDDIR=""
CEXTRA=""
GUARDSARM_CONF="guardsarm-manager.conf"
GUARDSARM_LOGFILE="guardsarm-manager.log"
GUARDSARM_LOGJSON="guardsarm-manager.json"

# Internal definitions
NEWCONFIG="./etc/guardsarm.mc"
PRECONFIG="./etc/PRECONFIG"

## Templates
TEMPLATE="./etc/templates"
ERROR="errors"
MSG="messages"

## Host output
OSSECMX="devmail.ossec.net mail is handled by 10 ossec.mooo.com."
OSSECMX2="devmail.ossec.net mail is handled (pri=10) by ossec.mooo.com"
OSSECMX3="devmail.ossec.net mail is handled by 10 ossec.mooo.COM."

## Predefined file
PREDEF_FILE="./etc/preloaded-vars.conf"

# Get number of processors
if [ -z "$THREADS" ]
then
    case $(uname) in
    Linux)
        THREADS=$(grep processor /proc/cpuinfo | wc -l)
        ;;
    Darwin)
        THREADS=$(sysctl -n hw.ncpu)
        ;;
    *)
        THREADS=1
    esac
fi
