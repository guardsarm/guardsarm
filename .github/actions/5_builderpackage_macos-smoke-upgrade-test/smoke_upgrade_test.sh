#!/bin/bash

# 1. Check if the GuardSarm agent is installed; if so, remove it.
# 2. Retrieve the latest version of the GuardSarm agent package.
# 3. Install the latest version of the GuardSarm agent.
# 4. Verify successful installation and start the agent.
# 5. Upgrade: Upgrade to the latest version using the builder package.
# 6. Verify successful upgrade and start the agent.
# 7. Verify the updated version.
# 8. Check if the guardsarm-agent service is running.


# Input:
old_package_url=$1
upgrade_version=$2
new_pkg=$3

ossec_path="/Library/Ossec"
guardsarm_control="$ossec_path/bin/guardsarm-control"

log_info() {
  echo "$(date '+%Y-%m-%d %H:%M:%S') [INFO] $1"
}

log_error() {
  echo "$(date '+%Y-%m-%d %H:%M:%S') [ERROR] $1" >&2
  exit 1
}

is_guardsarm_agent_installed() {
  if [ -d "$ossec_path" ]; then
    return 0
  else
    return 1
  fi
}

get_guardsarm_version(){
    if [ -f "$guardsarm_control" ]; then
        echo "$($guardsarm_control info -v)"
    else
        log_error "The script $guardsarm_control does not exist."
    fi
}

download_pkg(){
    local url=$1
    local output_file="/tmp/$(basename "$url")"
    curl -o "$output_file" "$url" && echo "$output_file"
}

uninstall_agent(){
    if launchctl list | grep -q "com.guardsarm.agent"; then
        launchctl unload /Library/LaunchDaemons/com.guardsarm.agent.plist
        /bin/rm -f /Library/LaunchDaemons/com.guardsarm.agent.plist
    else
        log_info "GuardSarm agent service is not currently loaded. Skipping unload."
    fi
    /bin/rm -r "$ossec_path"
    /bin/rm -rf /Library/StartupItems/GUARDSARM
    /usr/bin/dscl . -delete "/Users/guardsarm"
    /usr/bin/dscl . -delete "/Groups/guardsarm"
    /usr/sbin/pkgutil --forget com.guardsarm.pkg.guardsarm-agent
}

install_agent(){
    local pkg_file=$1

    echo "GUARDSARM_MANAGER='1.1.1.1'" > /tmp/guardsarm_envs && installer -pkg $pkg_file -target / | tee '/tmp/installer.log'
    launchctl load /Library/LaunchDaemons/com.guardsarm.agent.plist
    if grep -iqE "The (upgrade|install) was successful" /tmp/installer.log; then
        local version_installed=$(get_guardsarm_version)
        log_info "Installed version: $version_installed"
    else
        log_error "The installation could not be completed. The package will not be uploaded.";
    fi
}

start_guardsarm_agent(){
    $guardsarm_control start
}

main(){
    if [ -z "$old_package_url" ]; then
        log_error "Missing package URL. Usage: $0 <package_url> <expected_version_upgrade>"
    fi

    if [ -z "$new_pkg" ]; then
        log_error "Missing expected upgrade version. Usage: $0 <package_url> <expected_version_upgrade>"
    fi

    old_version=$(download_pkg "$old_package_url" || log_error "Failed to download package.")

    if is_guardsarm_agent_installed; then
        version_installed=$(get_guardsarm_version)
        log_info "Uninstalling guardsarm-agent: $version_installed"
        uninstall_agent
        log_info "GuardSarm successfully uninstalled."
    fi

    install_agent $old_version
    start_guardsarm_agent

    log_info "Perform upgrade"
    install_agent $new_pkg
    version_installed=$(get_guardsarm_version)
    version_installed="${version_installed#v}"
    start_guardsarm_agent

    if [ "$version_installed" != "$upgrade_version" ]; then
        log_error "Upgrade version $version_installed does not match expected $upgrade_version"
    fi

    if [ "$(/Library/Ossec/bin/guardsarm-control status|grep "is running" -c)" -ne "5" ]; then
        log_error "The service is not running for guardsarm version $version_installed"
    fi

    exit 0
}

main
