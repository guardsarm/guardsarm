<#
  GuardsArm ransomware canary guard — on-agent PREVENTION (Windows).

  Windows sibling of guardsarm-ransom-guard.py. Plants decoy "canary" files in the
  user-data folders, watches them with a FileSystemWatcher (instant, event-driven),
  and the moment a canary is modified/renamed/deleted it:
    1. Uses the Windows Restart Manager (RmGetList) to find WHICH process is holding
       the canary file open — the reliable way to identify the encrypting process.
    2. Kills that process (Stop-Process -Force) unless it is allow-listed.
    3. Emits an edr_malware canary detection (rule GuardsArm_Ransomware_Canary) to the
       agent EDR telemetry log -> Malware Center -> corr-ransom incident -> auto-isolate.
    4. Re-plants the canary.

  Run continuously from the agent command wodle (see install notes). Only ever touches
  its own canary files. Copyright (C) 2026, GuardsArm.
#>
[CmdletBinding()]
param([switch]$Once)

$ErrorActionPreference = 'Continue'
$AgentHome = if ($env:GS_AGENT_HOME) { $env:GS_AGENT_HOME } else { 'C:\Program Files (x86)\ossec-agent' }
$EdrLog    = if ($env:GS_EDR_LOG) { $env:GS_EDR_LOG } else { Join-Path $AgentHome 'logs\edr-telemetry.log' }
$Kill      = $env:GS_RANSOM_GUARD_KILL -notin @('0','false','no')
$Token     = 'GuardsArm_DO_NOT_REMOVE_canary'
$CanaryRule = 'GuardsArm_Ransomware_Canary'
$AllowList = @('System','Idle','wazuh-agent','guardsarm-agent','MsMpEng','SearchIndexer',
               'explorer','OneDrive','Dropbox','svchost','backup','csrss','wininit','services')

# Canary dirs: each user's profile data + common share roots (NOT Windows/ProgramData).
function Get-CanaryDirs {
    if ($env:GS_RANSOM_CANARY_DIRS) { return $env:GS_RANSOM_CANARY_DIRS -split ',' | Where-Object { $_ } }
    $dirs = @()
    Get-ChildItem 'C:\Users' -Directory -ErrorAction SilentlyContinue | ForEach-Object {
        foreach ($sub in 'Documents','Desktop','Pictures') {
            $p = Join-Path $_.FullName $sub
            if (Test-Path $p) { $dirs += $p }
        }
    }
    foreach ($share in (Get-SmbShare -ErrorAction SilentlyContinue | Where-Object { $_.Path -and $_.Name -notlike '*$' })) {
        if (Test-Path $share.Path) { $dirs += $share.Path }
    }
    return $dirs
}

$CanaryBody = ("GuardsArm ransomware canary. This decoy is monitored by the endpoint agent; " +
               "any modification triggers automatic containment. Do not encrypt/rename/delete.`n") * 40

function Get-CanaryPaths([string]$dir) {
    @((Join-Path $dir "!!!_0000_$Token.docx"), (Join-Path $dir "zzzz_9999_$Token.xlsx"))
}

function Plant-Canaries($dirs) {
    $planted = @()
    foreach ($d in $dirs) {
        foreach ($p in (Get-CanaryPaths $d)) {
            try {
                if (-not (Test-Path $p) -or ((Get-Content $p -Raw -ErrorAction SilentlyContinue) -ne $CanaryBody)) {
                    Set-Content -Path $p -Value $CanaryBody -NoNewline -Encoding UTF8 -ErrorAction Stop
                    (Get-Item $p -Force).Attributes = 'Normal'
                }
                $planted += $p
            } catch {}
        }
    }
    return $planted
}

# Restart Manager P/Invoke — returns the processes currently locking a file.
if (-not ([System.Management.Automation.PSTypeName]'GsRm').Type) {
Add-Type -Namespace '' -Name 'GsRm' -MemberDefinition @'
[StructLayout(LayoutKind.Sequential)] public struct RM_UNIQUE_PROCESS { public int dwProcessId; public System.Runtime.InteropServices.ComTypes.FILETIME ProcessStartTime; }
[DllImport("rstrtmgr.dll", CharSet=CharSet.Unicode)] public static extern int RmStartSession(out uint pSessionHandle, int dwSessionFlags, string strSessionKey);
[DllImport("rstrtmgr.dll", CharSet=CharSet.Unicode)] public static extern int RmRegisterResources(uint pSessionHandle, uint nFiles, string[] rgsFilenames, uint nApplications, RM_UNIQUE_PROCESS[] rgApplications, uint nServices, string[] rgsServiceNames);
[DllImport("rstrtmgr.dll")] public static extern int RmGetList(uint dwSessionHandle, out uint pnProcInfoNeeded, ref uint pnProcInfo, [In,Out] RM_PROCESS_INFO[] rgAffectedApps, ref uint lpdwRebootReasons);
[DllImport("rstrtmgr.dll")] public static extern int RmEndSession(uint pSessionHandle);
[StructLayout(LayoutKind.Sequential, CharSet=CharSet.Unicode)] public struct RM_PROCESS_INFO { public RM_UNIQUE_PROCESS Process; [MarshalAs(UnmanagedType.ByValTStr, SizeConst=256)] public string strAppName; [MarshalAs(UnmanagedType.ByValTStr, SizeConst=64)] public string strServiceShortName; public int ApplicationType; public uint AppStatus; public uint TSSessionId; [MarshalAs(UnmanagedType.Bool)] public bool bRestartable; }
'@
}

function Get-LockingProcesses([string]$path) {
    $pids = @()
    $sk = [Guid]::NewGuid().ToString()
    $h = [uint32]0
    if ([GsRm]::RmStartSession([ref]$h, 0, $sk) -ne 0) { return $pids }
    try {
        $null = [GsRm]::RmRegisterResources($h, 1, @($path), 0, $null, 0, $null)
        [uint32]$need = 0; [uint32]$got = 0; [uint32]$reason = 0
        $r = [GsRm]::RmGetList($h, [ref]$need, [ref]$got, $null, [ref]$reason)
        if ($need -gt 0) {
            $arr = New-Object 'GsRm+RM_PROCESS_INFO[]' $need
            $got = $need
            if ([GsRm]::RmGetList($h, [ref]$need, [ref]$got, $arr, [ref]$reason) -eq 0) {
                for ($i=0; $i -lt $got; $i++) { $pids += $arr[$i].Process.dwProcessId }
            }
        }
    } catch {} finally { [void][GsRm]::RmEndSession($h) }
    return $pids
}

function Emit-Canary([string]$canary, [string]$action, [string]$proc, $procId) {
    $ev = @{ collector = 'edr_malware'; data = @{ malware = @{
                rule = $CanaryRule; file = $canary
                detected = (Get-Date).ToUniversalTime().ToString('yyyy-MM-ddTHH:mm:ss') + 'Z'
                processName = $proc; pid = $procId; action = $action } } }
    try {
        $dir = Split-Path $EdrLog -Parent
        if (-not (Test-Path $dir)) { New-Item -ItemType Directory -Path $dir -Force | Out-Null }
        Add-Content -Path $EdrLog -Value ($ev | ConvertTo-Json -Compress -Depth 6) -Encoding UTF8
    } catch { Write-Warning "ransom-guard emit failed: $_" }
}

function Respond([string]$canary, [string]$action) {
    $procName = 'unknown'; $procId = $null
    foreach ($pid2 in (Get-LockingProcesses $canary)) {
        try { $p = Get-Process -Id $pid2 -ErrorAction Stop } catch { continue }
        if ($AllowList -contains $p.Name) { continue }
        $procName = $p.Name; $procId = $pid2
        if ($Kill) { try { Stop-Process -Id $pid2 -Force -ErrorAction Stop; Write-Host "ransom-guard: killed $($p.Name) ($pid2)" } catch {} }
    }
    Emit-Canary $canary $action $procName $procId
    Write-Host "ransom-guard: CANARY $action $canary -> $procName"
}

$dirs = Get-CanaryDirs
$paths = Plant-Canaries $dirs
Write-Host "ransom-guard: planted $($paths.Count) canaries across $($dirs.Count) dirs (kill=$Kill, once=$Once)"

if ($Once) {
    foreach ($p in $paths) {
        if (-not (Test-Path $p) -or ((Get-Content $p -Raw -ErrorAction SilentlyContinue) -ne $CanaryBody)) { Respond $p 'modified' }
    }
    return
}

# Event-driven watch on each canary directory.
$watchers = @()
foreach ($d in $dirs) {
    $w = New-Object System.IO.FileSystemWatcher $d
    $w.Filter = "*$Token*"
    $w.IncludeSubdirectories = $false
    $w.NotifyFilter = [System.IO.NotifyFilters]'FileName,LastWrite,Size'
    $w.EnableRaisingEvents = $true
    $action = { param($s,$e) Respond $e.FullPath $e.ChangeType.ToString(); Plant-Canaries (Get-CanaryDirs) | Out-Null }
    Register-ObjectEvent $w Changed -Action $action | Out-Null
    Register-ObjectEvent $w Deleted -Action $action | Out-Null
    Register-ObjectEvent $w Renamed -Action $action | Out-Null
    $watchers += $w
}
Write-Host "ransom-guard: watching $($watchers.Count) directories (Ctrl+C to stop)"
while ($true) { Start-Sleep -Seconds 5; Plant-Canaries $dirs | Out-Null }
