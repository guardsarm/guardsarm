#!/bin/sh

# GuardSarm Configuration File Generator
# Copyright (C) 2026 GuardSarm, Inc.
# November 24, 2016.
#
# Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.

# Looking up for the execution directory
SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)
# Change to the parent directory of src/init so relative paths work
cd "${SCRIPT_DIR}/../.."

. ${SCRIPT_DIR}/shared.sh
. ${SCRIPT_DIR}/inst-functions.sh

NEWCONFIG="./localfiles.temp"

if [ -r "$NEWCONFIG" ]; then
    rm "$NEWCONFIG"
fi

if [ "$#" = "1" ]; then
  INSTALLDIR="$1"
fi

echo "" >> $NEWCONFIG

echo "<guardsarm_config>" >> $NEWCONFIG

WriteLogs "add"

echo "</guardsarm_config>" >> $NEWCONFIG

cat "$NEWCONFIG"

rm "$NEWCONFIG"

exit 0
