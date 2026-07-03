#!/bin/sh

## Stop and remove application
sudo /Library/Ossec/bin/guardsarm-control stop
sudo /bin/rm -r /Library/Ossec*

# remove launchdaemons
/bin/rm -f /Library/LaunchDaemons/com.guardsarm.agent.plist

## remove StartupItems
/bin/rm -rf /Library/StartupItems/GUARDSARM

## Remove User and Groups
/usr/bin/dscl . -delete "/Users/guardsarm"
/usr/bin/dscl . -delete "/Groups/guardsarm"

/usr/sbin/pkgutil --forget com.guardsarm.pkg.guardsarm-agent
/usr/sbin/pkgutil --forget com.guardsarm.pkg.guardsarm-agent-etc

# In case it was installed via Puppet pkgdmg provider

if [ -e /var/db/.puppet_pkgdmg_installed_guardsarm-agent ]; then
    rm -f /var/db/.puppet_pkgdmg_installed_guardsarm-agent
fi

echo
echo "GuardSarm agent correctly removed from the system."
echo

exit 0
