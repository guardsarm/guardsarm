<#
  GuardsArm ransomware RECOVERY provisioning (Windows) — enable Volume Shadow Copy
  on user-data volumes and schedule periodic shadow copies, so
  guardsarm-restore-snapshot.ps1 can roll files back after a ransomware incident
  (isolate -> RESTORE, never pay).

  Idempotent: safe to re-run. Uses VSS (built into Windows) — no extra software.
  Sets a per-volume ShadowStorage cap and a scheduled task that creates a shadow
  copy of every fixed volume holding user data.

  Params (env or args, all optional):
    -IntervalHours  how often to snapshot           (default 4)
    -MaxSizePct     ShadowStorage cap per volume    (default 15%)
    -Volumes        drive letters to protect        (default: all fixed NTFS volumes)

  Copyright (C) 2026, GuardsArm.
#>
[CmdletBinding()]
param(
  [int]$IntervalHours = $(if ($env:GS_SNAPSHOT_INTERVAL_HOURS) { [int]$env:GS_SNAPSHOT_INTERVAL_HOURS } else { 4 }),
  [string]$MaxSizePct = $(if ($env:GS_SNAPSHOT_MAXPCT) { $env:GS_SNAPSHOT_MAXPCT } else { '15%' }),
  [string[]]$Volumes,
  [switch]$Run                       # -Run = take a snapshot now (scheduled invocation)
)
$ErrorActionPreference = 'Continue'
$GsRoot = if (Test-Path 'C:\Program Files (x86)\ossec-agent') { 'C:\Program Files (x86)\ossec-agent' } else { 'C:\Program Files (x86)\guardsarm-agent' }
$Log = Join-Path $GsRoot 'logs\snapshot-provision.log'
function Log($m) { $line = "{0} {1}" -f (Get-Date -Format s), $m; try { Add-Content -Path $Log -Value $line -EA SilentlyContinue } catch {}; Write-Output $line }

function Get-TargetVolumes {
  if ($Volumes) { return $Volumes }
  # all fixed NTFS volumes with a drive letter (where user data lives)
  Get-CimInstance Win32_Volume -EA SilentlyContinue |
    Where-Object { $_.DriveType -eq 3 -and $_.DriveLetter -and $_.FileSystem -eq 'NTFS' } |
    ForEach-Object { $_.DriveLetter }
}

function Ensure-ShadowStorage {
  foreach ($v in Get-TargetVolumes) {
    try {
      # cap shadow storage so snapshots can't fill the disk; self-hosted on same volume
      & vssadmin Resize ShadowStorage /For=$v /On=$v /MaxSize=$MaxSizePct 2>&1 |
        Out-String | ForEach-Object { if ($_ -match 'error|Error') { Log "resize $v -> $_" } }
      Log "shadowstorage ensured on $v (max $MaxSizePct)"
    } catch { Log "shadowstorage $v failed: $_" }
  }
}

function Snapshot-Now {
  $ok = 0
  foreach ($v in Get-TargetVolumes) {
    try {
      # WMI is the reliable programmatic way to create a shadow copy (vssadmin Create
      # is Server-only); ClientAccessible works on Win client + Server.
      $r = (Get-CimInstance -List Win32_ShadowCopy).Create("$v\", 'ClientAccessible')
      if ($r.ReturnValue -eq 0) { Log "shadow copy created on $v (id $($r.ShadowID))"; $ok++ }
      else { Log "shadow copy on $v returned $($r.ReturnValue)" }
    } catch { Log "shadow copy $v failed: $_" }
    # retention: keep the newest 24 shadow copies per volume
    try {
      $vol = (Get-CimInstance Win32_Volume -Filter "DriveLetter='$v'" -EA SilentlyContinue).DeviceID
      if ($vol) {
        Get-CimInstance Win32_ShadowCopy -EA SilentlyContinue |
          Where-Object { $_.VolumeName -eq $vol } |
          Sort-Object InstallDate -Descending | Select-Object -Skip 24 |
          ForEach-Object { $_ | Remove-CimInstance -EA SilentlyContinue; Log "pruned old shadow $($_.ID) on $v" }
      }
    } catch {}
  }
  return $ok
}

function Install-Schedule {
  $self = Join-Path $GsRoot 'active-response\bin\guardsarm-provision-snapshots.ps1'
  $action  = New-ScheduledTaskAction -Execute 'PowerShell.exe' `
             -Argument "-NonInteractive -ExecutionPolicy Bypass -File `"$self`" -Run"
  $trigger = New-ScheduledTaskTrigger -Once -At (Get-Date).AddMinutes(2) `
             -RepetitionInterval (New-TimeSpan -Hours $IntervalHours)
  $principal = New-ScheduledTaskPrincipal -UserId 'SYSTEM' -LogonType ServiceAccount -RunLevel Highest
  $set = New-ScheduledTaskSettingsSet -StartWhenAvailable -DontStopOnIdleEnd -ExecutionTimeLimit (New-TimeSpan -Minutes 30)
  try {
    Register-ScheduledTask -TaskName 'GuardsArm Ransomware Snapshot' -Action $action -Trigger $trigger `
      -Principal $principal -Settings $set -Force | Out-Null
    Log "scheduled task 'GuardsArm Ransomware Snapshot' installed (every ${IntervalHours}h)"
  } catch { Log "schedule install failed: $_" }
}

New-Item -ItemType Directory -Force -Path (Split-Path $Log) -EA SilentlyContinue | Out-Null
if ($Run) {
  $n = Snapshot-Now
  Log "scheduled run complete ($n volume(s) snapshotted)"
} else {
  Log 'provisioning ransomware-recovery shadow copies (VSS)'
  Ensure-ShadowStorage
  Install-Schedule
  Snapshot-Now | Out-Null
  Log 'provisioning complete; restore with active-response\bin\guardsarm-restore-snapshot.ps1'
}
