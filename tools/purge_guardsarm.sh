#!/usr/bin/env bash
# Copyright (C) 2026 GuardSarm, Inc.
# Purge script for GuardSarm
#
# Usage:
#   sudo ./purge_guardsarm.sh
#
# Behavior:
#   1. If guardsarm-agent or guardsarm-manager packages are installed, purge them.
#   2. Always perform a final cleanup of service units, users/groups and
#      installation directories.
#   3. If no packages are installed, treat the system as a source/manual
#      installation and perform the same full cleanup directly.

readonly DEFAULT_GUARDSARM_AGENT_DIR="/var/ossec"
readonly DEFAULT_GUARDSARM_MANAGER_DIR="/var/guardsarm-manager"

PKG_MANAGER=""
INSTALL_DIRS=()
PACKAGE_NAMES=()

require_root() {
    if [ "$(id -u)" -ne 0 ]; then
        echo "ERROR: This script must be run as root."
        echo "       Try: sudo $0"
        exit 1
    fi
}

append_install_dir() {
    local candidate="$1"
    local existing=""

    [ -n "$candidate" ] || return 0

    for existing in "${INSTALL_DIRS[@]}"; do
        [ "$existing" = "$candidate" ] && return 0
    done

    INSTALL_DIRS+=("$candidate")
}

init_install_dirs() {
    INSTALL_DIRS=()

    append_install_dir "${DEFAULT_GUARDSARM_MANAGER_DIR}"
    append_install_dir "${DEFAULT_GUARDSARM_AGENT_DIR}"
    append_install_dir "${GUARDSARM_HOME:-}"
    detect_install_dirs_from_services
}

detect_install_dirs_from_services() {
    local service_file=""
    local detected_dir=""

    for service_file in \
        /etc/systemd/system/guardsarm-manager.service \
        /etc/systemd/system/guardsarm-agent.service \
        /usr/lib/systemd/system/guardsarm-manager.service \
        /usr/lib/systemd/system/guardsarm-agent.service \
        /lib/systemd/system/guardsarm-manager.service \
        /lib/systemd/system/guardsarm-agent.service \
        /etc/init.d/guardsarm-manager \
        /etc/init.d/guardsarm-agent \
        /etc/rc.d/init.d/guardsarm-manager \
        /etc/rc.d/init.d/guardsarm-agent; do
        [ -f "${service_file}" ] || continue

        case "${service_file}" in
            *.service)
                detected_dir=$(
                    sed -n 's|^ExecStart=/usr/bin/env \(.*\)/bin/[^[:space:]]*control start$|\1|p' "${service_file}" |
                        head -n 1
                )
                ;;
            *)
                detected_dir=$(sed -n 's/^GUARDSARM_HOME=\(.*\)$/\1/p' "${service_file}" | head -n 1)
                ;;
        esac

        append_install_dir "${detected_dir}"
    done
}

stop_services() {
    local service_name=""
    local install_dir=""

    if command -v systemctl >/dev/null 2>&1; then
        for service_name in guardsarm-manager guardsarm-agent guardsarm-local; do
            systemctl stop "${service_name}" 2>/dev/null || true
            systemctl disable "${service_name}" 2>/dev/null || true
        done
    fi

    if command -v service >/dev/null 2>&1; then
        service guardsarm-manager stop 2>/dev/null || true
        service guardsarm-agent stop 2>/dev/null || true
        service guardsarm-local stop 2>/dev/null || true
    fi

    for install_dir in "${INSTALL_DIRS[@]}"; do
        [ -x "${install_dir}/bin/guardsarm-manager-control" ] && \
            "${install_dir}/bin/guardsarm-manager-control" stop 2>/dev/null || true
        [ -x "${install_dir}/bin/guardsarm-control" ] && \
            "${install_dir}/bin/guardsarm-control" stop 2>/dev/null || true
    done

    if command -v pkill >/dev/null 2>&1; then
        pkill -9 -f 'guardsarm-manager-authd|guardsarm-manager-remoted|guardsarm-manager-monitord|guardsarm-manager-modulesd|guardsarm-manager-analysisd|guardsarm-manager-db|guardsarm_manager_apid|guardsarm_manager_clusterd' 2>/dev/null || true
        pkill -9 -f 'guardsarm-agentd|guardsarm-syscheckd|guardsarm-logcollector|guardsarm-execd|guardsarm-modulesd' 2>/dev/null || true
    fi
}

unmount_dev_proc() {
    local install_dir=""
    local proc_dir=""

    command -v mountpoint >/dev/null 2>&1 || return 0

    for install_dir in "${INSTALL_DIRS[@]}"; do
        proc_dir="${install_dir}/proc"

        if [ -d "${proc_dir}" ] && mountpoint -q "${proc_dir}"; then
            echo " - Unmounting ${proc_dir}"
            umount "${proc_dir}" 2>/dev/null || true
        fi
    done
}

detect_packages() {
    local packages=""

    PKG_MANAGER=""
    PACKAGE_NAMES=()

    if command -v dpkg-query >/dev/null 2>&1 && command -v apt-get >/dev/null 2>&1; then
        PKG_MANAGER="apt"
        packages=$(
            dpkg-query -W -f='${Package}\t${Status}\n' 2>/dev/null |
                awk '$4=="installed" && ($1=="guardsarm-manager" || $1=="guardsarm-agent") {print $1}' |
                tr '\n' ' '
        )
    elif command -v rpm >/dev/null 2>&1; then
        if command -v dnf >/dev/null 2>&1; then
            PKG_MANAGER="dnf"
        elif command -v yum >/dev/null 2>&1; then
            PKG_MANAGER="yum"
        elif command -v zypper >/dev/null 2>&1; then
            PKG_MANAGER="zypper"
        else
            PKG_MANAGER="rpm"
        fi

        packages=$(
            rpm -qa 2>/dev/null |
                grep -E '^guardsarm-(manager|agent)(-|$)' |
                tr '\n' ' ' || true
        )
    fi

    [ -n "${packages}" ] && read -r -a PACKAGE_NAMES <<<"${packages}"
}

purge_packages() {
    [ "${#PACKAGE_NAMES[@]}" -gt 0 ] || return 0

    echo " - Removing packages: ${PACKAGE_NAMES[*]}"

    case "${PKG_MANAGER}" in
        apt)
            DEBIAN_FRONTEND=noninteractive apt-get purge --no-auto-remove -y "${PACKAGE_NAMES[@]}" 2>/dev/null || true
            DEBIAN_FRONTEND=noninteractive apt-get autoremove -y 2>/dev/null || true
            apt-get autoclean -y 2>/dev/null || true
            ;;
        dnf)
            dnf remove -y "${PACKAGE_NAMES[@]}" 2>/dev/null || true
            ;;
        yum)
            yum remove -y "${PACKAGE_NAMES[@]}" 2>/dev/null || true
            ;;
        zypper)
            zypper --non-interactive remove -u "${PACKAGE_NAMES[@]}" 2>/dev/null || true
            ;;
        rpm)
            rpm -e --nodeps "${PACKAGE_NAMES[@]}" 2>/dev/null || true
            ;;
    esac
}

remove_install_dirs() {
    local install_dir=""

    for install_dir in "${INSTALL_DIRS[@]}"; do
        if [ -d "${install_dir}" ]; then
            echo " - Removing ${install_dir}"
            rm -rf -- "${install_dir}"
        fi
    done
}

remove_service_units() {
    local service_dir=""
    local init_dir=""
    local init_name=""

    if [ -f /etc/rc.local ]; then
        sed -i '/guardsarm-control start/d;/guardsarm-manager-control start/d;/guardsarm-local-control start/d' /etc/rc.local 2>/dev/null || true
    fi

    for init_dir in /etc/init.d /etc/rc0.d /etc/rc1.d /etc/rc2.d /etc/rc3.d /etc/rc4.d /etc/rc5.d /etc/rc6.d; do
        [ -d "${init_dir}" ] || continue
        for init_name in guardsarm-manager guardsarm-agent guardsarm-local; do
            rm -f \
                "${init_dir}/${init_name}" \
                "${init_dir}"/S??"${init_name}" \
                "${init_dir}"/K??"${init_name}" 2>/dev/null || true
        done
    done

    if command -v systemctl >/dev/null 2>&1; then
        for service_dir in /etc/systemd/system /usr/lib/systemd/system /lib/systemd/system; do
            [ -d "${service_dir}" ] || continue
            find "${service_dir}" -maxdepth 2 \( \
                -name 'guardsarm-manager.service' -o \
                -name 'guardsarm-agent.service' -o \
                -name 'guardsarm-local.service' \
            \) -exec rm -f {} + 2>/dev/null || true
        done

        systemctl daemon-reload 2>/dev/null || true
        systemctl reset-failed 2>/dev/null || true
    fi
}

remove_misc_files() {
    [ -e /etc/ossec-init.conf ] && rm -f /etc/ossec-init.conf
}

remove_users_groups() {
    local user_name=""
    local group_name=""

    for user_name in guardsarm guardsarm-manager; do
        if id "${user_name}" >/dev/null 2>&1; then
            echo " - Removing user: ${user_name}"
            userdel "${user_name}" 2>/dev/null || userdel -r "${user_name}" 2>/dev/null || true
        fi
    done

    for group_name in guardsarm guardsarm-manager; do
        if getent group "${group_name}" >/dev/null 2>&1; then
            echo " - Removing group: ${group_name}"
            groupdel "${group_name}" 2>/dev/null || true
        fi
    done
}

cleanup_system() {
    unmount_dev_proc
    remove_install_dirs
    remove_service_units
    remove_misc_files
    remove_users_groups
}

main() {
    echo ""
    echo " GuardSarm purge script"
    echo " =================="
    echo ""

    require_root
    init_install_dirs

    echo " - Stopping GuardSarm services..."
    stop_services

    echo " - Checking for installed packages..."
    detect_packages

    if [ "${#PACKAGE_NAMES[@]}" -gt 0 ]; then
        echo " - Package-managed installation detected."
        purge_packages

        echo " - Removing remaining GuardSarm files..."
        cleanup_system
        echo ""
        echo " - GuardSarm packages purged successfully."
    else
        echo " - No installed guardsarm-manager/guardsarm-agent packages detected."
        echo " - Performing source/manual cleanup."
        cleanup_system
        echo ""
        echo " - Source/manual installation removed successfully."
    fi

    echo ""
    echo " GuardSarm purge completed."
    echo ""
}

main "$@"
