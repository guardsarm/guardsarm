<#
  GuardsArm — install/update Sysmon with the recommended config.
  Copyright (C) 2026, GuardSarm, Inc.

  Sysmon feeds the Microsoft-Windows-Sysmon/Operational channel, which the agent
  already tails (localfile in gsmsec.conf) and the manager decodes
  (load_sysmon_decoders.py). This script:
    - installs Sysmon if absent (using a local Sysmon64.exe, or downloads it),
    - (re)applies sysmonconfig.xml if Sysmon is already installed,
    - is idempotent and reversible (-Uninstall).

  USAGE (elevated):
    .\install-sysmon.ps1                       # install/update with bundled config
    .\install-sysmon.ps1 -SysmonExe C:\t\Sysmon64.exe
    .\install-sysmon.ps1 -Config .\sysmonconfig.xml
    .\install-sysmon.ps1 -Uninstall
    .\install-sysmon.ps1 -AllowDownload        # fetch Sysmon64.exe from Sysinternals
#>
[CmdletBinding()]
param(
    [string]$SysmonExe = "",
    [string]$Config = "",
    [switch]$AllowDownload,
    [switch]$Uninstall
)
$ErrorActionPreference = "Stop"
$here = Split-Path -Parent $MyInvocation.MyCommand.Path
if (-not $Config) { $Config = Join-Path $here "sysmonconfig.xml" }

function Test-Admin {
    $id = [Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()
    return $id.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
}
if (-not (Test-Admin)) { throw "Must run elevated (Sysmon install requires Administrator)." }

function Get-SysmonCmd {
    # 1. A Sysmon binary bundled next to this script (shipped in the agent MSI). This is
    #    what makes the MSI custom action argument-free and work fully offline.
    foreach ($c in @("Sysmon64.exe","Sysmon.exe")) {
        $bundled = Join-Path $here $c
        if (Test-Path $bundled) { return $bundled }
    }
    # 2. On PATH.
    foreach ($c in @("Sysmon64.exe","Sysmon.exe")) {
        $p = Get-Command $c -ErrorAction SilentlyContinue
        if ($p) { return $p.Source }
    }
    # 3. Installed Sysmon copies itself to System32.
    foreach ($p in @("$env:WINDIR\Sysmon64.exe","$env:WINDIR\Sysmon.exe")) {
        if (Test-Path $p) { return $p }
    }
    return $null
}

function Test-SysmonInstalled {
    return [bool](Get-Service -Name "Sysmon64","Sysmon" -ErrorAction SilentlyContinue)
}

if ($Uninstall) {
    $exe = if ($SysmonExe) { $SysmonExe } else { Get-SysmonCmd }
    if (-not $exe) { Write-Host "Sysmon binary not found; nothing to uninstall."; exit 0 }
    Write-Host "Uninstalling Sysmon..."
    & $exe -u force
    exit $LASTEXITCODE
}

if (-not (Test-Path $Config)) { throw "Config not found: $Config" }

# Resolve the Sysmon binary
if (-not $SysmonExe) { $SysmonExe = Get-SysmonCmd }
if (-not $SysmonExe) {
    if ($AllowDownload) {
        $dst = Join-Path $env:TEMP "Sysmon64.exe"
        Write-Host "Downloading Sysmon64.exe from Sysinternals..."
        # live.sysinternals.com serves the current signed binary
        Invoke-WebRequest -Uri "https://live.sysinternals.com/Sysmon64.exe" -OutFile $dst -UseBasicParsing
        $SysmonExe = $dst
    } else {
        throw "Sysmon binary not found. Provide -SysmonExe <path>, or pass -AllowDownload to fetch it from Sysinternals."
    }
}
Write-Host "Sysmon binary: $SysmonExe"
Write-Host "Config       : $Config"

if (Test-SysmonInstalled) {
    Write-Host "Sysmon already installed -> applying config (-c)..."
    & $SysmonExe -c $Config
} else {
    Write-Host "Installing Sysmon with config (-accepteula -i)..."
    & $SysmonExe -accepteula -i $Config
}
$rc = $LASTEXITCODE
if ($rc -ne 0) { throw "Sysmon returned exit code $rc" }

# Confirm the channel exists (the agent localfile will now receive events)
$log = Get-WinEvent -ListLog "Microsoft-Windows-Sysmon/Operational" -ErrorAction SilentlyContinue
if ($log) {
    Write-Host ("OK: Sysmon operational channel present (IsEnabled={0}, records={1})." -f $log.IsEnabled, $log.RecordCount) -ForegroundColor Green
    Write-Host "The GuardsArm agent's <localfile>Microsoft-Windows-Sysmon/Operational</localfile> will now forward these events."
} else {
    Write-Host "WARNING: Sysmon operational channel not found after install." -ForegroundColor Yellow
}
exit 0
