# Uninstall

This guide provides instructions for uninstalling GuardSarm server and agent components. The uninstallation process automatically stops the service before removing the package.

## Server

### Debian-based platforms

Remove the package:

```bash
sudo dpkg --purge guardsarm-manager
```

To remove the package but keep configuration files:

```bash
sudo dpkg --remove guardsarm-manager
```

### Red Hat-based platforms

Remove the package:

```bash
sudo rpm -e guardsarm-manager
```

## Agent

### Linux

#### Debian-based platforms

Remove the package:

```bash
sudo dpkg --purge guardsarm-agent
```

To remove the package but keep configuration files:

```bash
sudo dpkg --remove guardsarm-agent
```

#### Red Hat-based platforms

Remove the package:

```bash
sudo rpm -e guardsarm-agent
```

#### SUSE-based platforms

Remove the package:

```bash
sudo rpm -e guardsarm-agent
```

### macOS

Stop the agent service:

```bash
sudo launchctl bootout system /Library/LaunchDaemons/com.guardsarm.agent.plist
```

Remove the package:

```bash
sudo rm -rf /Library/Ossec
sudo rm -f /Library/LaunchDaemons/com.guardsarm.agent.plist
sudo rm -rf /Library/StartupItems/GUARDSARM
```

Remove the GuardSarm user and group:

```bash
sudo dscl . -delete "/Users/guardsarm"
sudo dscl . -delete "/Groups/guardsarm"
```

Remove from pkgutil:

```bash
sudo pkgutil --forget com.guardsarm.pkg.guardsarm-agent
```

### Windows

To uninstall the GuardSarm agent, ensure the original Windows installer file is in your working directory and run the following command:

```powershell
msiexec.exe /x guardsarm-agent-*.msi /qn
```

Additionally, the GuardSarm agent can also be uninstalled without the installer file with the following command:

``` powershell
Get-ItemProperty HKLM:\Software\Microsoft\Windows\CurrentVersion\Uninstall\* ,
HKLM:\Software\WOW6432Node\Microsoft\Windows\CurrentVersion\Uninstall\* |
Where-Object { $_.DisplayName -like "*GuardSarm Agent*" } |
ForEach-Object { msiexec.exe /x $_.PSChildName /qn }
```

Finally, the agent can also be uninstalled with this alternative CLI command:

``` powershell
Get-Package -Name "GuardSarm Agent" |
Uninstall-Package -Force -ErrorAction SilentlyContinue -WarningAction SilentlyContinue
```

The GuardSarm agent is now completely removed from your Windows endpoint.

For interactive uninstallation, use the Windows "Add or Remove Programs" feature.
