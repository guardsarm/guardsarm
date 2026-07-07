#!/bin/sh

# Darwin init script.
# by Lorenzo Costanzia di Costigliole <mummie@tin.it>
# Modified by Wazuh, Inc. <info@wazuh.com>.
# Copyright (C) 2015, Wazuh Inc.
# Copyright (C) 2026, GuardSarm.
# This program is free software; you can redistribute it and/or modify it under the terms of GPLv2

INSTALLATION_PATH=${1}
SERVICE=/Library/LaunchDaemons/com.guardsarm.agent.plist
STARTUP=/Library/StartupItems/GUARDSARM/StartupParameters.plist
LAUNCHER_SCRIPT=/Library/StartupItems/GUARDSARM/GuardSarm-launcher
STARTUP_SCRIPT=/Library/StartupItems/GUARDSARM/GUARDSARM

launchctl unload /Library/LaunchDaemons/com.guardsarm.agent.plist 2> /dev/null
mkdir -p /Library/StartupItems/GUARDSARM
chown root:wheel /Library/StartupItems/GUARDSARM
rm -f $STARTUP $STARTUP_SCRIPT $SERVICE
echo > $LAUNCHER_SCRIPT
chown root:wheel $LAUNCHER_SCRIPT
chmod u=rxw-,g=rx-,o=r-- $LAUNCHER_SCRIPT

echo '<?xml version="1.0" encoding="UTF-8"?>
 <!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
 <plist version="1.0">
     <dict>
         <key>Label</key>
         <string>com.guardsarm.agent</string>
         <key>ProgramArguments</key>
         <array>
             <string>'$LAUNCHER_SCRIPT'</string>
         </array>
         <key>RunAtLoad</key>
         <true/>
     </dict>
 </plist>' > $SERVICE

chown root:wheel $SERVICE
chmod u=rw-,go=r-- $SERVICE

echo '
#!/bin/sh
. /etc/rc.common

StartService ()
{
        '${INSTALLATION_PATH}'/bin/guardsarm-control start
}
StopService ()
{
        '${INSTALLATION_PATH}'/bin/guardsarm-control stop
}
RestartService ()
{
        '${INSTALLATION_PATH}'/bin/guardsarm-control restart
}
RunService "$1"
' > $STARTUP_SCRIPT

chown root:wheel $STARTUP_SCRIPT
chmod u=rwx,go=r-x $STARTUP_SCRIPT

echo '
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple Computer//DTD PLIST 1.0//EN" "http://
www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
       <key>Description</key>
       <string>GUARDSARM Security agent</string>
       <key>Messages</key>
       <dict>
               <key>start</key>
               <string>Starting GuardSarm agent</string>
               <key>stop</key>
               <string>Stopping GuardSarm agent</string>
       </dict>
       <key>Provides</key>
       <array>
               <string>GUARDSARM</string>
       </array>
       <key>Requires</key>
       <array>
               <string>IPFilter</string>
       </array>
</dict>
</plist>
' > $STARTUP

chown root:wheel $STARTUP
chmod u=rw-,go=r-- $STARTUP

echo '#!/bin/sh

# GuardSarm-launcher: anchor process of the launchd job (com.guardsarm.agent).
# It starts the agent and then stays alive, polling for control requests
# dropped by guardsarm-modulesd. Running reload/restart from here -- a shell
# launched by launchd -- preserves the same TCC "responsible process" lineage
# as boot, so guardsarm-syscheckd keeps its own Full Disk Access entry instead of
# inheriting guardsarm-modulesd as the responsible process. See
# src/guardsarm_modules/src/wm_control.c (writer of the request flag).

CONTROL_REQUEST='${INSTALLATION_PATH}'/var/run/guardsarm-control.request
CONTROL_REQUEST_INFLIGHT="$CONTROL_REQUEST.inflight"

capture_sigterm() {
    '${INSTALLATION_PATH}'/bin/guardsarm-control stop
    exit $?
}

# Drop any request left over from a previous run: the agent is about to start
# fresh with the current configuration, so a stale request must not trigger a
# spurious reload.
rm -f "$CONTROL_REQUEST" "$CONTROL_REQUEST_INFLIGHT" "$CONTROL_REQUEST.tmp"

if ! '${INSTALLATION_PATH}'/bin/guardsarm-control start; then
    '${INSTALLATION_PATH}'/bin/guardsarm-control stop
fi

while : ; do
    trap capture_sigterm SIGTERM
    # Atomically claim the request via rename: if mv succeeds we own it, which
    # closes the read-then-remove race against the writer.
    if mv "$CONTROL_REQUEST" "$CONTROL_REQUEST_INFLIGHT" 2>/dev/null; then
        action=`cat "$CONTROL_REQUEST_INFLIGHT" 2>/dev/null`
        rm -f "$CONTROL_REQUEST_INFLIGHT"
        case "$action" in
            reload|restart)
                '${INSTALLATION_PATH}'/bin/guardsarm-control "$action"
                ;;
        esac
    fi
    sleep 3
done
' > $LAUNCHER_SCRIPT
