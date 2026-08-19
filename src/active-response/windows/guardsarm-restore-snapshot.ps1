<#
  GuardsArm ransomware RECOVERY on Windows — restore user data from a Volume Shadow
  Copy (item e). Windows sibling of guardsarm-restore-snapshot.py.

  After a corr-ransom incident is contained, this finds the most recent VSS shadow
  copy and restores the target path from it, so isolation is followed by RESTORE.
  Wired as the `restore-snapshot` active-response on Windows.

  DRY-RUN by default; pass -Apply to perform the copy-back.
  Usage:  guardsarm-restore-snapshot.ps1 -Target 'C:\Users' [-Apply]
  Copyright (C) 2026, GuardsArm.
#>
[CmdletBinding()]
param([string]$Target = 'C:\Users', [switch]$Apply, [switch]$List)

$ErrorActionPreference = 'Continue'

function Get-LatestShadow([string]$volume) {
    $shadows = Get-CimInstance Win32_ShadowCopy -ErrorAction SilentlyContinue |
        Where-Object { $_.VolumeName -and $volume.StartsWith($_.VolumeName.Substring(0,2), 'InvariantCultureIgnoreCase') } |
        Sort-Object InstallDate -Descending
    if (-not $shadows) { $shadows = Get-CimInstance Win32_ShadowCopy -ErrorAction SilentlyContinue | Sort-Object InstallDate -Descending }
    return $shadows | Select-Object -First 1
}

if ($List) {
    Get-CimInstance Win32_ShadowCopy -ErrorAction SilentlyContinue |
        Select-Object DeviceObject, InstallDate, VolumeName | Format-Table -AutoSize
    if (-not (Get-CimInstance Win32_ShadowCopy -ErrorAction SilentlyContinue)) {
        Write-Output (@{ error = 'no VSS shadow copies present';
            recommend = 'enable System Protection / scheduled VSS snapshots so ransomware is recoverable (vssadmin, or a backup product)' } | ConvertTo-Json)
    }
    return
}

$vol = (Split-Path $Target -Qualifier) + '\'
$shadow = Get-LatestShadow $vol
if (-not $shadow) {
    Write-Output (@{ error = 'no VSS shadow copy found for restore'; target = $Target;
        recommend = 'enable System Protection / scheduled snapshots' } | ConvertTo-Json)
    exit 2
}

# The shadow copy is addressable via its DeviceObject (\\?\GLOBALROOT\Device\HarddiskVolumeShadowCopyN).
$rel = $Target.Substring(2).TrimStart('\')       # strip 'C:'
$src = Join-Path ($shadow.DeviceObject + '\') $rel

$result = @{ mechanism = 'vss'; shadow = $shadow.DeviceObject; installDate = $shadow.InstallDate.ToString('o'); source = $src; target = $Target }
if (-not $Apply) {
    $result.action = 'dry-run'
    $result.note = "would copy-back $Target from shadow $($shadow.DeviceObject)"
} else {
    try {
        # robocopy mirrors the clean files back over the encrypted ones (/XO keeps newer, but
        # here we prefer shadow originals -> /IS overwrites); adjust switches per policy.
        $link = "$env:TEMP\gs_vss_$([guid]::NewGuid().ToString('N'))"
        cmd /c "mklink /d `"$link`" `"$($shadow.DeviceObject)\`"" | Out-Null
        $rc = (Start-Process robocopy -ArgumentList "`"$link\$rel`" `"$Target`" /E /IS /R:1 /W:1" -Wait -PassThru -NoNewWindow).ExitCode
        cmd /c "rmdir `"$link`"" | Out-Null
        $result.action = 'restore'; $result.robocopyRc = $rc
    } catch { $result.action = 'error'; $result.note = $_.Exception.Message }
}
Write-Output ($result | ConvertTo-Json -Depth 5)
