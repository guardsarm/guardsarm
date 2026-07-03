#!/bin/bash
set -euo pipefail

log_error() {
    echo "Error: $1"
    exit 1
}

log_info() {
    echo "[INFO] $1"
}

log_success() {
    echo "[SUCCESS] $1"
}

set_utils(){
    if compgen -G "/packages/*.deb" >/dev/null 2>&1; then
        install="dpkg --install"
        check_package_status='dpkg-query -W'
        package_extension="deb"
    elif compgen -G "/packages/*.rpm" >/dev/null 2>&1; then
        install="rpm -ivh --force --ignorearch"
        check_package_status='rpm -q'
        package_extension="rpm"
    else
        log_error "Couldn't find type of system"
    fi
}

install_package(){
    local package_path=$1

    if [ -z "$package_path" ]; then
        log_error "No package found to install"
    fi

    if [ ! -f "$package_path" ]; then
        log_error "Package file not found: $package_path"
    fi

    log_info "Installing package: $package_path"

    GUARDSARM_MANAGER="1.2.3.4" $install "$package_path"

    # Verify installation
    if $check_package_status "guardsarm-agent" >/dev/null 2>&1; then
        log_success "Package installed successfully"
    else
        log_error "Package installation verification failed"
    fi
}

test_daemons(){
    local daemons="guardsarm-modulesd guardsarm-logcollector guardsarm-syscheckd guardsarm-agentd guardsarm-execd"

    log_info "Testing daemons with -t flag (configuration test)..."

    for daemon in $daemons; do
        log_info "Testing ${daemon}..."
        if ! "/var/ossec/bin/${daemon}" -t 2>&1; then
            log_error "${daemon} -t failed"
        fi
        log_success "${daemon} -t passed"
    done
}

start_agent(){
    log_info "Starting GuardSarm agent..."
    if ! /var/ossec/bin/guardsarm-control start 2>&1; then
        log_error "Failed to start GuardSarm agent"
    fi
    log_success "GuardSarm agent started"

    log_info "Waiting for agent to initialize (10 seconds)..."
    sleep 10
}

verify_agent_running(){
    log_info "Verifying agent is running..."
    if ! /var/ossec/bin/guardsarm-control status 2>&1; then
        log_error "Agent status check failed"
    fi
    log_success "Agent is running"
}

stop_agent(){
    log_info "Stopping GuardSarm agent..."
    if ! /var/ossec/bin/guardsarm-control stop 2>&1; then
        log_error "Failed to stop GuardSarm agent"
    fi
    log_success "GuardSarm agent stopped successfully"
}

main() {
    log_info "=== GuardSarm Agent Startup Smoke Test ==="

    set_utils

    # Find the package recursively (artifacts may be in subdirectories)
    package_name=$(find /packages -type f -name "guardsarm-agent*.$package_extension" ! -name "*dbg*" ! -name "*debug*" ! -name "*debuginfo*" | head -n1)

    if [ -z "$package_name" ]; then
        log_error "No suitable package found in /packages/"
    fi

    log_info "Found package: $package_name"

    # Run smoke test steps
    install_package "$package_name"
    test_daemons
    start_agent
    verify_agent_running
    stop_agent

    log_success "=== Smoke test completed successfully ==="
}

main
