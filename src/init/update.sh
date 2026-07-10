#!/bin/sh

# Copyright (C) 2026 GuardSarm, Inc.
# Shell script update functions for GuardSarm
# Author: Daniel B. Cid <daniel.cid@gmail.com>

FALSE="false"
TRUE="true"

doUpdatecleanup()
{
    if [ "X$INSTALLDIR" = "X" ]; then
        echo "# ($FUNCNAME) ERROR: The variable INSTALLDIR wasn't set." 1>&2
        echo "${FALSE}"
        return 1;
    fi

    # Checking if the directory is valid.
    _dir_pattern_update="^/[-a-zA-Z0-9/\.-]{3,128}$"
    echo $INSTALLDIR | grep -E "$_dir_pattern_update" > /dev/null 2>&1
    if [ ! $? = 0 ]; then
        echo "# ($FUNCNAME) ERROR: directory name ($INSTALLDIR) doesn't match the pattern $_dir_pattern_update" 1>&2
        echo "${FALSE}"
        return 1;
    fi
}

##########
# Checks if GuardSarm is installed by taking the installdir from the services
# files (if exists) and taking into account the installation type.
#
# getPreinstalledDirByType()
##########
getPreinstalledDirByType()
{
    SERVICE_UNIT_PATH=""

    # Checking for Systemd
    if hash ps 2>&1 > /dev/null && hash grep 2>&1 > /dev/null && [ -n "$(ps -e | egrep ^\ *1\ .*systemd$)" ]; then

        SED_EXTRACT_PREINSTALLEDDIR="s|^ExecStart=/usr/bin/env \\(.*\\)/bin/[^[:space:]]*control start$|\\1|p"

        if [ "X$pidir_service_name" = "Xguardsarm-manager" ]; then #manager or agent
            type="manager"
        else
            type="agent"
        fi

        # Get the unit file and extract the GuardSarm home path
        PREINSTALLEDDIR=$(systemctl cat guardsarm-${type}.service 2>/dev/null | sed -n "${SED_EXTRACT_PREINSTALLEDDIR}")
        if [ -n "${PREINSTALLEDDIR}" ]; then
            if [ -d "${PREINSTALLEDDIR}" ]; then
                return 0;
            else
                PREINSTALL_DETECTION_ERROR="Detected guardsarm-${type}.service metadata pointing to '${PREINSTALLEDDIR}', but that directory does not exist."
                return 2;
            fi
        fi

        # If fail, find the service file
        # RHEL 8 / Amazon / openSUSE Tumbleweed the services should be installed in /usr/lib/systemd/system/
        if [ -f /usr/lib/systemd/system/guardsarm-${type}.service ]; then
            SERVICE_UNIT_PATH=/usr/lib/systemd/system/guardsarm-${type}.service
        fi
        # Others
        if [ -f /etc/systemd/system/guardsarm-${type}.service ]; then
            SERVICE_UNIT_PATH=/etc/systemd/system/guardsarm-${type}.service
        fi

        if [ -f "$SERVICE_UNIT_PATH" ]; then
            PREINSTALLEDDIR=$(sed -n "${SED_EXTRACT_PREINSTALLEDDIR}" "${SERVICE_UNIT_PATH}")
            if [ -d "$PREINSTALLEDDIR" ]; then
                return 0;
            elif [ -n "${PREINSTALLEDDIR}" ]; then
                PREINSTALL_DETECTION_ERROR="Detected service file '${SERVICE_UNIT_PATH}' pointing to '${PREINSTALLEDDIR}', but that directory does not exist."
                return 2;
            else
                PREINSTALL_DETECTION_ERROR="Detected service file '${SERVICE_UNIT_PATH}', but no installation directory could be extracted from it."
                return 2;
            fi
        else
            return 1;
        fi
    fi
    # Checking for Redhat system.
    if [ -r "/etc/redhat-release" ]; then
        if [ -d /etc/rc.d/init.d ]; then
            if [ -f /etc/rc.d/init.d/${pidir_service_name} ]; then
                PREINSTALLEDDIR=`sed -n 's/^GUARDSARM_HOME=\(.*\)$/\1/p' /etc/rc.d/init.d/${pidir_service_name}`
                if [ -d "$PREINSTALLEDDIR" ]; then
                    return 0;
                else
                    return 1;
                fi
            else
                return 1;
            fi
        fi
    fi
    # Checking for Gentoo
    if [ -r "/etc/gentoo-release" ]; then
        if [ -f /etc/init.d/${pidir_service_name} ]; then
            PREINSTALLEDDIR=`sed -n 's/^GUARDSARM_HOME=\(.*\)$/\1/p' /etc/init.d/${pidir_service_name}`
            if [ -d "$PREINSTALLEDDIR" ]; then
                return 0;
            else
                return 1;
            fi
        else
            return 1;
        fi
    fi
    # Checking for Suse
    if [ -r "/etc/SuSE-release" ]; then
        if [ -f /etc/init.d/${pidir_service_name} ]; then
            PREINSTALLEDDIR=`sed -n 's/^GUARDSARM_HOME=\(.*\)$/\1/p' /etc/init.d/${pidir_service_name}`
            if [ -d "$PREINSTALLEDDIR" ]; then
                return 0;
            else
                return 1;
            fi
        else
            return 1;
        fi
    fi
    # Checking for Slackware
    if [ -r "/etc/slackware-version" ]; then
        if [ -f /etc/rc.d/rc.${pidir_service_name} ]; then
            PREINSTALLEDDIR=`sed -n 's/^GUARDSARM_HOME=\(.*\)$/\1/p' /etc/rc.d/rc.${pidir_service_name}`
            if [ -d "$PREINSTALLEDDIR" ]; then
                return 0;
            else
                return 1;
            fi
        else
            return 1;
        fi
    fi
    # Checking for Darwin
    if [ "X${NUNAME}" = "XDarwin" ]; then
        if [ -f /Library/StartupItems/GUARDSARM/GUARDSARM ]; then
            PREINSTALLEDDIR=`sed -n 's/^ *//; s|^\\s*\\(.*\\)/bin/[^[:space:]]*control start$|\\1|p' /Library/StartupItems/GUARDSARM/GUARDSARM`
            if [ -d "$PREINSTALLEDDIR" ]; then
                return 0;
            else
                return 1;
            fi
        else
            return 1;
        fi
    fi
    # Checking for BSD
    if [ "X${UN}" = "XOpenBSD" -o "X${UN}" = "XNetBSD" -o "X${UN}" = "XFreeBSD" -o "X${UN}" = "XDragonFly" ]; then
        # Checking for the presence of the control script on rc.local
        grep -E 'guardsarm(-manager)?-control' /etc/rc.local > /dev/null 2>&1
        if [ $? = 0 ]; then
            PREINSTALLEDDIR=`sed -n 's|^\\(.*\\)/bin/[^[:space:]]*control start$|\\1|p' /etc/rc.local`
            if [ -d "$PREINSTALLEDDIR" ]; then
                return 0;
            else
                return 1;
            fi
        else
            return 1;
        fi
    elif [ "X${NUNAME}" = "XLinux" ]; then
        # Checking for Linux
        if [ -e "/etc/rc.d/rc.local" ]; then
            grep -E 'guardsarm(-manager)?-control' /etc/rc.d/rc.local > /dev/null 2>&1
            if [ $? = 0 ]; then
                PREINSTALLEDDIR=`sed -n 's|^\\(.*\\)/bin/[^[:space:]]*control start$|\\1|p' /etc/rc.d/rc.local`
                if [ -d "$PREINSTALLEDDIR" ]; then
                    return 0;
                else
                    return 1;
                fi
            else
                return 1;
            fi
        # Checking for Linux (SysV)
        elif [ -d "/etc/rc.d/init.d" ]; then
            if [ -f /etc/rc.d/init.d/${pidir_service_name} ]; then
                PREINSTALLEDDIR=`sed -n 's/^GUARDSARM_HOME=\(.*\)$/\1/p' /etc/rc.d/init.d/${pidir_service_name}`
                if [ -d "$PREINSTALLEDDIR" ]; then
                    return 0;
                else
                    return 1;
                fi
            else
                return 1;
            fi
        # Checking for Debian (Ubuntu or derivative)
        elif [ -d "/etc/init.d" -a -f "/usr/sbin/update-rc.d" ]; then
            if [ -f /etc/init.d/${pidir_service_name} ]; then
                PREINSTALLEDDIR=`sed -n 's/^GUARDSARM_HOME=\(.*\)$/\1/p' /etc/init.d/${pidir_service_name}`
                if [ -d "$PREINSTALLEDDIR" ]; then
                    return 0;
                else
                    return 1;
                fi
            else
                return 1;
            fi
        fi
    fi

    return 1;
}

##########
# Checks if GuardSarm is installed in the specified path by searching for the control binary.
#
# isGuardSarmInstalled()
##########
isGuardSarmInstalled()
{
    if [ -f "${1}/bin/guardsarm-manager-control" ]; then
        return 0;
    elif [ -f "${1}/bin/guardsarm-control" ]; then
        return 0;
    else
        return 1;
    fi
}

##########
# Checks if GuardSarm is installed by trying with each installation type.
# If it finds an installation, it sets the PREINSTALLEDDIR variable.
# After that it checks if GuardSarm is truly installed there, if it is installed it returns TRUE.
# If it isn't installed continue searching in other installation types and replacing PREINSTALLEDDIR variable.
# It returns FALSE if GuardSarm isn't installed in any of this.
#
# getPreinstalledDir()
##########
getPreinstalledDir()
{
    # Getting preinstalled dir for GuardSarm manager installations
    pidir_service_name="guardsarm-manager"
    if getPreinstalledDirByType && isGuardSarmInstalled $PREINSTALLEDDIR; then
        return 0;
    fi

    # Getting preinstalled dir for GuardSarm agent installations
    pidir_service_name="guardsarm-agent"
    if getPreinstalledDirByType && isGuardSarmInstalled $PREINSTALLEDDIR; then
        return 0;
    fi

    return 1;
}

getPreinstalledType()
{
    if [ "X$PREINSTALLEDDIR" = "X" ]; then
        getPreinstalledDir
    fi

    if [ -f "$PREINSTALLEDDIR/bin/guardsarm-manager-control" ]; then
        TYPE=`$PREINSTALLEDDIR/bin/guardsarm-manager-control info -t`
    else
        TYPE=`$PREINSTALLEDDIR/bin/guardsarm-control info -t`
    fi

    case "$TYPE" in
        server)
            TYPE="manager"
            ;;
    esac

    echo $TYPE
    return 0;
}

getPreinstalledVersion()
{
    if [ "X$PREINSTALLEDDIR" = "X" ]; then
        getPreinstalledDir
    fi

    if [ -f "$PREINSTALLEDDIR/bin/guardsarm-manager-control" ]; then
        VERSION=`$PREINSTALLEDDIR/bin/guardsarm-manager-control info -v`
    else
        VERSION=`$PREINSTALLEDDIR/bin/guardsarm-control info -v`
    fi

    echo $VERSION
}

getPreinstalledName()
{
    NAME="GuardSarm"
    echo $NAME
}

UpdateStartGUARDSARM()
{
    if [ "X$TYPE" = "X" ]; then
        getPreinstalledType
    fi

    if [ "X$TYPE" != "Xagent" ]; then
        TYPE="manager"
    fi

    if [ `stat /proc/1/exe 2> /dev/null | grep "systemd" | wc -l` -ne 0 ]; then
        systemctl start guardsarm-$TYPE
    elif [ `stat /proc/1/exe 2> /dev/null | grep "init.d" | wc -l` -ne 0 ]; then
        service guardsarm-$TYPE start
    else
        # Considering that this function is only used after finishing the installation
        # the INSTALLDIR variable is always set. It could have either the default value,
        # or a value equals to the PREINSTALLEDDIR, or a value specified by the user.
        # The last two possibilities are set in the setInstallDir function.
        if [ -f "$INSTALLDIR/bin/guardsarm-manager-control" ]; then
            $INSTALLDIR/bin/guardsarm-manager-control start
        else
            $INSTALLDIR/bin/guardsarm-control start
        fi
    fi
}

UpdateStopGUARDSARM()
{
    MAJOR_VERSION=`echo ${VERSION} | cut -f1 -d'.' | cut -f2 -d'v'`

    if [ "X$TYPE" = "X" ]; then
        getPreinstalledType
    fi

    if [ "X$TYPE" != "Xagent" ]; then
        TYPE="manager"
        if [ $MAJOR_VERSION -ge 4 ]; then
            EMBEDDED_API_INSTALLED=1
        fi
    fi

    if [ `stat /proc/1/exe 2> /dev/null | grep "systemd" | wc -l` -ne 0 ]; then
        systemctl stop guardsarm-$TYPE
    elif [ `stat /proc/1/exe 2> /dev/null | grep "init.d" | wc -l` -ne 0 ]; then
        service guardsarm-$TYPE stop
    fi

    # Make sure GuardSarm is stopped
    if [ "X$PREINSTALLEDDIR" = "X" ]; then
        getPreinstalledDir
    fi

    if [ -f "$PREINSTALLEDDIR/bin/guardsarm-manager-control" ]; then
        $PREINSTALLEDDIR/bin/guardsarm-manager-control stop > /dev/null 2>&1
    else
        $PREINSTALLEDDIR/bin/guardsarm-control stop > /dev/null 2>&1
    fi

    sleep 2

   # We also need to remove all syscheck queue file (format changed)
    if [ "X$VERSION" = "X0.9-3" ]; then
        rm -f $PREINSTALLEDDIR/queue/syscheck/* > /dev/null 2>&1
        rm -f $PREINSTALLEDDIR/queue/agent-info/* > /dev/null 2>&1
    fi
    rm -rf $PREINSTALLEDDIR/framework/* > /dev/null 2>&1
    rm -f $PREINSTALLEDDIR/wodles/aws/aws > /dev/null 2>&1 # this script has been renamed
    rm -f $PREINSTALLEDDIR/wodles/aws/aws.py > /dev/null 2>&1 # this script has been renamed

    # Deleting plain-text agent information if exists (it was migrated to GuardSarm DB in v4.1)
    if [ -d "$PREINSTALLEDDIR/queue/agent-info" ]; then
        rm -rf $PREINSTALLEDDIR/queue/agent-info > /dev/null 2>&1
    fi

    # Deleting plain-text rootcheck information if exists (it was migrated to GuardSarm DB in v4.1)
    if [ -d "$PREINSTALLEDDIR/queue/rootcheck" ]; then
        rm -rf $PREINSTALLEDDIR/queue/rootcheck > /dev/null 2>&1
    fi
}

UpdateOldVersions()
{

    # If it is GuardSarm 2.0 or newer, exit
    if [ "X$USER_OLD_NAME" = "XGuardSarm" ]; then
        return
    fi

    if [ "X$PREINSTALLEDDIR" != "X" ]; then
        getPreinstalledDir
    fi

    GUARDSARM_CONF_FILE="$PREINSTALLEDDIR/etc/${GUARDSARM_CONF:-gsmsec.conf}"
    GUARDSARM_CONF_FILE_ORIG="$PREINSTALLEDDIR/etc/${GUARDSARM_CONF:-gsmsec.conf}.orig"

    # config file -> config file.orig
    cp -pr $GUARDSARM_CONF_FILE $GUARDSARM_CONF_FILE_ORIG

    # Delete old service
    if [ -f /etc/init.d/ossec ]; then
        rm /etc/init.d/ossec
    fi

    if [ ! "$INSTYPE" = "agent" ]; then
        # New manager config by default
        ./src/init/gen_guardsarm.sh conf "manager" $DIST_NAME $DIST_VER > $GUARDSARM_CONF_FILE
    else
        # New agent config by default
        ./src/init/gen_guardsarm.sh conf "agent" $DIST_NAME $DIST_VER > $GUARDSARM_CONF_FILE
        # Replace IP
        ./src/init/replace_manager_ip.sh $GUARDSARM_CONF_FILE_ORIG $GUARDSARM_CONF_FILE
        ./src/init/add_localfiles.sh $PREINSTALLEDDIR >> $GUARDSARM_CONF_FILE
    fi
}
