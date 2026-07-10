#!/bin/sh
# Copyright (C) 2026 GuardSarm, Inc.
# Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.

WPYTHON_BIN="framework/python/bin/python3"

SCRIPT_PATH_NAME="$0"

DIR_NAME="$(cd $(dirname ${SCRIPT_PATH_NAME}); pwd -P)"
SCRIPT_NAME="$(basename ${SCRIPT_PATH_NAME})"

case ${DIR_NAME} in
    */active-response/bin | */wodles*)
        if [ -z "${GUARDSARM_PATH}" ]; then
            GUARDSARM_PATH="$(cd ${DIR_NAME}/../..; pwd)"
        fi

        PYTHON_SCRIPT="${DIR_NAME}/${SCRIPT_NAME}.py"
    ;;
    */bin)
        if [ -z "${GUARDSARM_PATH}" ]; then
            GUARDSARM_PATH="$(cd ${DIR_NAME}/..; pwd)"
        fi

        PYTHON_SCRIPT="${GUARDSARM_PATH}/api/scripts/$(echo ${SCRIPT_NAME} | sed 's/\-/_/g').py"
    ;;
esac


${GUARDSARM_PATH}/${WPYTHON_BIN} ${PYTHON_SCRIPT} $@