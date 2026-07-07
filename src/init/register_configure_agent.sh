#!/bin/bash

# Copyright (C) 2015, Wazuh Inc.
# Copyright (C) 2026, GuardSarm.
#
# This program is free software; you can redistribute it
# and/or modify it under the terms of the GNU General Public
# License (version 2) as published by the FSF - Free Software
# Foundation.

# Global variables
INSTALLDIR=${1}
CONF_FILE="${INSTALLDIR}/etc/ossec.conf"
TMP_ENROLLMENT="${INSTALLDIR}/tmp/enrollment-configuration"
TMP_SERVER="${INSTALLDIR}/tmp/server-configuration"
GUARDSARM_REGISTRATION_PASSWORD_PATH="etc/authd.pass"
GUARDSARM_MACOS_AGENT_DEPLOYMENT_VARS="/tmp/guardsarm_envs"


# Set default sed alias
sed="sed -ri"

# Update the value of a XML tag inside the guardsarm configuration file
edit_value_tag() {

    file=""

    if [ -z "$3" ]; then
        file="${CONF_FILE}"
    else
        file="${TMP_ENROLLMENT}"
    fi

    if [ -n "$1" ] && [ -n "$2" ]; then
        start_config="$(grep -n "<$1>" "${file}" | cut -d':' -f 1)"
        end_config="$(grep -n "</$1>" "${file}" | cut -d':' -f 1)"
        if [ -z "${start_config}" ] && [ -z "${end_config}" ] && [ "${file}" = "${TMP_ENROLLMENT}" ]; then
            echo "      <$1>$2</$1>" >> "${file}"
        else
            ${sed} "s#<$1>.*</$1>#<$1>$2</$1>#g" "${file}"
        fi
    fi

    if [ "$?" != "0" ]; then
        echo "$(date '+%Y/%m/%d %H:%M:%S') Error updating $2 with variable $1." >> "${INSTALLDIR}/logs/ossec.log"
    fi

}

delete_blank_lines() {

    file=$1
    ${sed} '/^$/d' "${file}"

}

delete_auto_enrollment_tag() {

    # Delete the configuration tag if its value is empty
    # This will allow using the default value
    ${sed} "s#.*<$1>.*</$1>.*##g" "${TMP_ENROLLMENT}"

    cat -s "${TMP_ENROLLMENT}" > "${TMP_ENROLLMENT}.tmp"
    mv "${TMP_ENROLLMENT}.tmp" "${TMP_ENROLLMENT}"

}

# Change address block of the guardsarm configuration file
add_adress_block() {

    # Remove both manager and legacy server configuration blocks
    ${sed} "/<manager>/,/\/manager>/d; /<server>/,/\/server>/d" "${CONF_FILE}"

    # Write the client configuration block
    for i in "${!ADDRESSES[@]}";
    do
        {
            echo "    <manager>"
            echo "      <address>${ADDRESSES[i]}</address>"
            echo "      <port>1514</port>"
            echo "    </manager>"
        } >> "${TMP_SERVER}"
    done

    ${sed} "/<client>/r ${TMP_SERVER}" "${CONF_FILE}"

    rm -f "${TMP_SERVER}"

}

add_parameter () {

    if [ -n "$3" ]; then
        OPTIONS="$1 $2 $3"
    fi
    echo "${OPTIONS}"

}

get_deprecated_vars () {

    if [ -n "${GUARDSARM_MANAGER_IP}" ] && [ -z "${GUARDSARM_MANAGER}" ]; then
        GUARDSARM_MANAGER=${GUARDSARM_MANAGER_IP}
    fi
    if [ -n "${GUARDSARM_AUTHD_SERVER}" ] && [ -z "${GUARDSARM_REGISTRATION_SERVER}" ]; then
        GUARDSARM_REGISTRATION_SERVER=${GUARDSARM_AUTHD_SERVER}
    fi
    if [ -n "${GUARDSARM_AUTHD_PORT}" ] && [ -z "${GUARDSARM_REGISTRATION_PORT}" ]; then
        GUARDSARM_REGISTRATION_PORT=${GUARDSARM_AUTHD_PORT}
    fi
    if [ -n "${GUARDSARM_PASSWORD}" ] && [ -z "${GUARDSARM_REGISTRATION_PASSWORD}" ]; then
        GUARDSARM_REGISTRATION_PASSWORD=${GUARDSARM_PASSWORD}
    fi
    if [ -n "${GUARDSARM_NOTIFY_TIME}" ] && [ -z "${GUARDSARM_KEEP_ALIVE_INTERVAL}" ]; then
        GUARDSARM_KEEP_ALIVE_INTERVAL=${GUARDSARM_NOTIFY_TIME}
    fi
    if [ -n "${GUARDSARM_CERTIFICATE}" ] && [ -z "${GUARDSARM_REGISTRATION_CA}" ]; then
        GUARDSARM_REGISTRATION_CA=${GUARDSARM_CERTIFICATE}
    fi
    if [ -n "${GUARDSARM_PEM}" ] && [ -z "${GUARDSARM_REGISTRATION_CERTIFICATE}" ]; then
        GUARDSARM_REGISTRATION_CERTIFICATE=${GUARDSARM_PEM}
    fi
    if [ -n "${GUARDSARM_KEY}" ] && [ -z "${GUARDSARM_REGISTRATION_KEY}" ]; then
        GUARDSARM_REGISTRATION_KEY=${GUARDSARM_KEY}
    fi
    if [ -n "${GUARDSARM_GROUP}" ] && [ -z "${GUARDSARM_AGENT_GROUP}" ]; then
        GUARDSARM_AGENT_GROUP=${GUARDSARM_GROUP}
    fi

}

set_vars () {

    export GUARDSARM_MANAGER
    export GUARDSARM_MANAGER_PORT
    export GUARDSARM_REGISTRATION_SERVER
    export GUARDSARM_REGISTRATION_PORT
    export GUARDSARM_REGISTRATION_PASSWORD
    export GUARDSARM_KEEP_ALIVE_INTERVAL
    export GUARDSARM_TIME_RECONNECT
    export GUARDSARM_REGISTRATION_CA
    export GUARDSARM_REGISTRATION_CERTIFICATE
    export GUARDSARM_REGISTRATION_KEY
    export GUARDSARM_AGENT_NAME
    export GUARDSARM_AGENT_GROUP
    export ENROLLMENT_DELAY
    # The following variables are yet supported but all of them are deprecated
    export GUARDSARM_MANAGER_IP
    export GUARDSARM_NOTIFY_TIME
    export GUARDSARM_AUTHD_SERVER
    export GUARDSARM_AUTHD_PORT
    export GUARDSARM_PASSWORD
    export GUARDSARM_GROUP
    export GUARDSARM_CERTIFICATE
    export GUARDSARM_KEY
    export GUARDSARM_PEM

    if [ -r "${GUARDSARM_MACOS_AGENT_DEPLOYMENT_VARS}" ]; then
        . ${GUARDSARM_MACOS_AGENT_DEPLOYMENT_VARS}
        rm -rf "${GUARDSARM_MACOS_AGENT_DEPLOYMENT_VARS}"
    fi

}

unset_vars() {

    vars=(GUARDSARM_MANAGER_IP GUARDSARM_MANAGER_PORT GUARDSARM_NOTIFY_TIME \
          GUARDSARM_TIME_RECONNECT GUARDSARM_AUTHD_SERVER GUARDSARM_AUTHD_PORT GUARDSARM_PASSWORD \
          GUARDSARM_AGENT_NAME GUARDSARM_GROUP GUARDSARM_CERTIFICATE GUARDSARM_KEY GUARDSARM_PEM \
          GUARDSARM_MANAGER GUARDSARM_REGISTRATION_SERVER GUARDSARM_REGISTRATION_PORT \
          GUARDSARM_REGISTRATION_PASSWORD GUARDSARM_KEEP_ALIVE_INTERVAL GUARDSARM_REGISTRATION_CA \
          GUARDSARM_REGISTRATION_CERTIFICATE GUARDSARM_REGISTRATION_KEY GUARDSARM_AGENT_GROUP \
          ENROLLMENT_DELAY)

    for var in "${vars[@]}"; do
        unset "${var}"
    done

}

# Function to convert strings to lower version
tolower () {

    echo "$1" | tr '[:upper:]' '[:lower:]'

}


# Add auto-enrollment configuration block
add_auto_enrollment () {

    start_config="$(grep -n "<enrollment>" "${CONF_FILE}" | cut -d':' -f 1)"
    end_config="$(grep -n "</enrollment>" "${CONF_FILE}" | cut -d':' -f 1)"
    if [ -n "${start_config}" ] && [ -n "${end_config}" ]; then
        start_config=$(( start_config + 1 ))
        end_config=$(( end_config - 1 ))
        sed -n "${start_config},${end_config}p" "${INSTALLDIR}/etc/ossec.conf" >> "${TMP_ENROLLMENT}"
    else
        # Write the client configuration block
        {
            echo "    <enrollment>"
            echo "      <enabled>yes</enabled>"
            echo "      <manager_address>MANAGER_IP</manager_address>"
            echo "      <port>1515</port>"
            echo "      <agent_name>agent</agent_name>"
            echo "      <groups>Group1</groups>"
            echo "      <server_ca_path>/path/to/server_ca</server_ca_path>"
            echo "      <agent_certificate_path>/path/to/agent.cert</agent_certificate_path>"
            echo "      <agent_key_path>/path/to/agent.key</agent_key_path>"
            echo "      <authorization_pass_path>/path/to/authd.pass</authorization_pass_path>"
            echo "      <delay_after_enrollment>20</delay_after_enrollment>"
            echo "    </enrollment>"
        } >> "${TMP_ENROLLMENT}"
    fi

}

# Add the auto_enrollment block to the configuration file
concat_conf() {

    ${sed} "/<\/auto_restart>/r ${TMP_ENROLLMENT}" "${CONF_FILE}"

    rm -f "${TMP_ENROLLMENT}"

}

# Set autoenrollment configuration
set_auto_enrollment_tag_value () {

    tag="$1"
    value="$2"

    if [ -n "${value}" ]; then
        edit_value_tag "${tag}" "${value}" "auto_enrollment"
    else
        delete_auto_enrollment_tag "${tag}" "auto_enrollment"
    fi

}

# Main function the script begin here
main () {

    uname_s=$(uname -s)

    # Check what kind of system we are working with
    if [ "${uname_s}" = "Darwin" ]; then
        sed="sed -ire"
        set_vars
    fi

    get_deprecated_vars

    if [ -n "${GUARDSARM_MANAGER}" ]; then
        if [ ! -f "${INSTALLDIR}/logs/ossec.log" ]; then
            touch -f "${INSTALLDIR}/logs/ossec.log"
            chmod 660 "${INSTALLDIR}/logs/ossec.log"
            chown root:guardsarm "${INSTALLDIR}/logs/ossec.log"
        fi

        # Check if multiples IPs are defined in variable GUARDSARM_MANAGER
        ADDRESSES=( ${GUARDSARM_MANAGER//,/ } )

        add_adress_block
    fi

    edit_value_tag "port" "${GUARDSARM_MANAGER_PORT}"

    if [ -n "${GUARDSARM_REGISTRATION_SERVER}" ] || [ -n "${GUARDSARM_REGISTRATION_PORT}" ] || [ -n "${GUARDSARM_REGISTRATION_CA}" ] || [ -n "${GUARDSARM_REGISTRATION_CERTIFICATE}" ] || [ -n "${GUARDSARM_REGISTRATION_KEY}" ] || [ -n "${GUARDSARM_AGENT_NAME}" ] || [ -n "${GUARDSARM_AGENT_GROUP}" ] || [ -n "${ENROLLMENT_DELAY}" ] || [ -n "${GUARDSARM_REGISTRATION_PASSWORD}" ]; then
        add_auto_enrollment
        set_auto_enrollment_tag_value "manager_address" "${GUARDSARM_REGISTRATION_SERVER}"
        set_auto_enrollment_tag_value "port" "${GUARDSARM_REGISTRATION_PORT}"
        set_auto_enrollment_tag_value "server_ca_path" "${GUARDSARM_REGISTRATION_CA}"
        set_auto_enrollment_tag_value "agent_certificate_path" "${GUARDSARM_REGISTRATION_CERTIFICATE}"
        set_auto_enrollment_tag_value "agent_key_path" "${GUARDSARM_REGISTRATION_KEY}"
        set_auto_enrollment_tag_value "authorization_pass_path" "${GUARDSARM_REGISTRATION_PASSWORD_PATH}"
        set_auto_enrollment_tag_value "agent_name" "${GUARDSARM_AGENT_NAME}"
        set_auto_enrollment_tag_value "groups" "${GUARDSARM_AGENT_GROUP}"
        set_auto_enrollment_tag_value "delay_after_enrollment" "${ENROLLMENT_DELAY}"
        delete_blank_lines "${TMP_ENROLLMENT}"
        concat_conf
    fi


    if [ -n "${GUARDSARM_REGISTRATION_PASSWORD}" ]; then
        echo "${GUARDSARM_REGISTRATION_PASSWORD}" > "${INSTALLDIR}/${GUARDSARM_REGISTRATION_PASSWORD_PATH}"
        chmod 640 "${INSTALLDIR}"/"${GUARDSARM_REGISTRATION_PASSWORD_PATH}"
        chown root:guardsarm "${INSTALLDIR}"/"${GUARDSARM_REGISTRATION_PASSWORD_PATH}"
    fi

    # Options to be modified in guardsarm configuration file
    edit_value_tag "notify_time" "${GUARDSARM_KEEP_ALIVE_INTERVAL}"
    edit_value_tag "time-reconnect" "${GUARDSARM_TIME_RECONNECT}"

    unset_vars

}

# Start script execution
main "$@"
