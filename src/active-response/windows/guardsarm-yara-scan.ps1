# GuardsArm on-agent YARA file scanner (Windows).
#
# The Linux/macOS agents run guardsarm-yara-scan.py (shells out to the `yara` CLI),
# but Windows agents ship no Python and no `yara` binary — so this PowerShell variant
# drives the bundled, statically-linked yara64.exe over the watched directories,
# computes each matched file's SHA256, and emits an edr_malware detection to the EDR
# telemetry log (forwarded to the manager, decoded to the `security` category and
# surfaced in the Malware Center). Deduplicates by rule:path so a standing file
# alerts once. Output shape is identical to the Python scanner so apply_edr_remote
# routes it the same way.
#
# Scheduled by the manager shared agent.conf (2h guardsarm-yara-scan command wodle).
# Copyright (C) 2026, GuardsArm.
[CmdletBinding()]
param([switch]$Once)

$ErrorActionPreference = 'Continue'

$AgentHome = if ($env:GS_AGENT_HOME) { $env:GS_AGENT_HOME }
             elseif (Test-Path 'C:\Program Files (x86)\gsmsec-agent') { 'C:\Program Files (x86)\gsmsec-agent' }
             else { 'C:\Program Files (x86)\ossec-agent' }

$ScriptDir = if ($PSScriptRoot) { $PSScriptRoot } else { Split-Path -Parent $MyInvocation.MyCommand.Path }

# yara64.exe is bundled alongside this script (active-response\bin); fall back to PATH.
$Yara = if (Test-Path (Join-Path $ScriptDir 'yara64.exe')) { Join-Path $ScriptDir 'yara64.exe' }
        elseif (Get-Command yara64.exe -ErrorAction SilentlyContinue) { 'yara64.exe' }
        elseif (Get-Command yara.exe   -ErrorAction SilentlyContinue) { 'yara.exe' }
        else { $null }

# Rule pack: env override -> etc\yara (Linux-parity path) -> co-located with the script.
$Rules = if ($env:GS_YARA_RULES) { $env:GS_YARA_RULES }
         elseif (Test-Path (Join-Path $AgentHome 'etc\yara\guardsarm-rules.yar')) { Join-Path $AgentHome 'etc\yara\guardsarm-rules.yar' }
         else { Join-Path $ScriptDir 'guardsarm-rules.yar' }

$EdrLog = if ($env:GS_EDR_LOG) { $env:GS_EDR_LOG } else { Join-Path $AgentHome 'logs\edr-telemetry.log' }
$State  = Join-Path $AgentHome 'var\yara-seen.json'

# Watch dirs: env override (semicolon-separated) or the high-risk write locations
# (each user's Downloads/Desktop/Documents/Temp + system temp + web roots + shares).
function Get-WatchDirs {
    if ($env:GS_YARA_DIRS) { return $env:GS_YARA_DIRS -split ';' | Where-Object { $_ } }
    $dirs = New-Object System.Collections.Generic.List[string]
    Get-ChildItem 'C:\Users' -Directory -ErrorAction SilentlyContinue | ForEach-Object {
        foreach ($sub in 'Downloads','Desktop','Documents','AppData\Local\Temp') {
            $p = Join-Path $_.FullName $sub
            if (Test-Path $p) { $dirs.Add($p) }
        }
    }
    foreach ($p in 'C:\Windows\Temp','C:\ProgramData','C:\Temp','C:\inetpub\wwwroot') {
        if (Test-Path $p) { $dirs.Add($p) }
    }
    foreach ($share in (Get-SmbShare -ErrorAction SilentlyContinue | Where-Object { $_.Path -and $_.Name -notlike '*$' })) {
        if (Test-Path $share.Path) { $dirs.Add($share.Path) }
    }
    return $dirs
}

function Load-Seen {
    try {
        if (Test-Path $State) {
            $a = Get-Content $State -Raw -ErrorAction Stop | ConvertFrom-Json
            $h = @{}; foreach ($k in $a) { $h[$k] = $true }; return $h
        }
    } catch {}
    return @{}
}

function Save-Seen($seen) {
    try {
        $dir = Split-Path $State -Parent
        if (-not (Test-Path $dir)) { New-Item -ItemType Directory -Path $dir -Force | Out-Null }
        @($seen.Keys) | ConvertTo-Json -Compress | Set-Content -Path $State -Encoding UTF8
    } catch {}
}

function Sha256([string]$path) {
    try { return (Get-FileHash -Algorithm SHA256 -LiteralPath $path -ErrorAction Stop).Hash.ToLower() }
    catch { return '' }
}

function Emit([string]$rule, [string]$file, [string]$hash) {
    $ev = @{ collector = 'edr_malware'; data = @{ malware = @{
                rule = $rule; file = $file; sha256 = $hash
                detected = (Get-Date).ToUniversalTime().ToString('yyyy-MM-ddTHH:mm:ss') + 'Z' } } }
    try {
        $dir = Split-Path $EdrLog -Parent
        if (-not (Test-Path $dir)) { New-Item -ItemType Directory -Path $dir -Force | Out-Null }
        # UTF-8 WITHOUT BOM: the EDR log is parsed as JSON-per-line; a BOM (which
        # Windows PowerShell's `Add-Content -Encoding UTF8` prepends on file creation)
        # would corrupt the first record if the scanner is ever the first writer.
        $json = ($ev | ConvertTo-Json -Compress -Depth 6) + "`n"
        [System.IO.File]::AppendAllText($EdrLog, $json, (New-Object System.Text.UTF8Encoding($false)))
    } catch { Write-Warning "yara-scan emit failed: $_" }
}

function Invoke-Scan {
    if (-not $Yara)            { Write-Warning "yara-scan: yara64.exe not found"; return 0 }
    if (-not (Test-Path $Rules)) { Write-Warning "yara-scan: rules not found ($Rules)"; return 0 }
    $seen = Load-Seen
    $hits = 0
    foreach ($d in (Get-WatchDirs)) {
        if (-not (Test-Path $d)) { continue }
        # -r recursive, -w suppress warnings, -f fast matching. Output: "RULENAME <path>".
        $out = & $Yara -r -w -f $Rules $d 2>$null
        foreach ($line in $out) {
            if (-not $line) { continue }
            $idx = $line.IndexOf(' ')
            if ($idx -lt 1) { continue }
            $rule = $line.Substring(0, $idx).Trim()
            $path = $line.Substring($idx + 1).Trim()
            if (-not $path) { continue }
            $key = "$rule`:$path"
            if ($seen.ContainsKey($key)) { continue }
            $seen[$key] = $true
            Emit $rule $path (Sha256 $path)
            $hits++
        }
    }
    Save-Seen $seen
    return $hits
}

$n = Invoke-Scan
Write-Host "yara-scan: $n new detection(s)"
