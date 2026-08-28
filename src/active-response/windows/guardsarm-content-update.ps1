# GuardsArm on-agent CONTENT auto-updater (Windows, pull model).
#
# Keeps the agent's DATA/CONTENT (YARA rules, active-response .ps1 scripts) current
# WITHOUT reinstalling the MSI. Pulls a SIGNED manifest + changed files from the
# content channel, verifies the manifest's RSA-SHA256 signature against a public key
# baked into the agent (.NET XML form — PowerShell 5.1 has no PEM importer), and
# applies only the changed files in place (atomic write + replace). No restart.
#
# Same hardening as the Linux updater: signature required (invalid -> abort), each
# file's sha256 pinned in the signed manifest and re-checked, target paths confined
# to an allow-listed set of subdirs under the agent home (no arbitrary write).
# Scheduled by the shared agent.conf command wodle. Copyright (C) 2026, GuardsArm.
[CmdletBinding()]
param()
$ErrorActionPreference = 'Continue'

$AgentHome = if ($env:GS_AGENT_HOME) { $env:GS_AGENT_HOME }
             elseif (Test-Path 'C:\Program Files (x86)\gsmsec-agent') { 'C:\Program Files (x86)\gsmsec-agent' }
             else { 'C:\Program Files (x86)\ossec-agent' }
$Base    = if ($env:GS_CONTENT_URL) { $env:GS_CONTENT_URL.TrimEnd('/') } else { 'https://packages.guardsarm.com/content' }
$Channel = if ($env:GS_CONTENT_CHANNEL) { $env:GS_CONTENT_CHANNEL } else { 'stable' }
$Url     = "$Base/$Channel/windows"
$PubXml  = if ($env:GS_CONTENT_PUBKEY) { $env:GS_CONTENT_PUBKEY }
           elseif (Test-Path (Join-Path $AgentHome 'etc\guardsarm-content-update.pub.xml')) { Join-Path $AgentHome 'etc\guardsarm-content-update.pub.xml' }
           else { Join-Path $AgentHome 'active-response\bin\guardsarm-content-update.pub.xml' }
$State   = Join-Path $AgentHome 'var\content-update.state'
$Log     = Join-Path $AgentHome 'logs\content-update.log'
$AllowedPrefixes = @('etc/yara/', 'etc/decoders/', 'etc/rules/', 'etc/lists/',
                     'active-response/bin/', 'ruleset/', 'etc/guardsarm-content-update.pub.xml',
                     'etc/wpk_root.pem')   # WPK remote-upgrade trust anchor (no reinstall)

function Write-Log([string]$m) {
    $line = (Get-Date).ToUniversalTime().ToString('yyyy-MM-ddTHH:mm:ssZ') + ' ' + $m
    try {
        $dir = Split-Path $Log -Parent
        if (-not (Test-Path $dir)) { New-Item -ItemType Directory -Path $dir -Force | Out-Null }
        [System.IO.File]::AppendAllText($Log, $line + "`n", (New-Object System.Text.UTF8Encoding($false)))
    } catch {}
    Write-Host $line
}

function Get-Url([string]$rel, [string]$dest) {
    [Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12
    $wc = New-Object System.Net.WebClient
    $wc.Headers.Add('User-Agent', 'guardsarm-content-update')
    try { $wc.DownloadFile("$Url/$rel", $dest) } finally { $wc.Dispose() }
}

function Get-Sha256([string]$path) {
    return (Get-FileHash -Algorithm SHA256 -LiteralPath $path).Hash.ToLower()
}

function Test-Signature([byte[]]$data, [byte[]]$sig) {
    if (-not (Test-Path $PubXml)) { Write-Log "ERROR: content public key missing ($PubXml)"; return $false }
    try {
        $rsa = New-Object System.Security.Cryptography.RSACryptoServiceProvider
        $rsa.FromXmlString((Get-Content -Raw $PubXml))
        return $rsa.VerifyData($data, 'SHA256', $sig)
    } catch { Write-Log "ERROR: signature verify failed: $($_.Exception.Message)"; return $false }
}

function Resolve-SafeTarget([string]$rel) {
    $rel = ($rel -replace '\\', '/').TrimStart('/')
    if (($rel -split '/') -contains '..') { return $null }
    $ok = $false
    foreach ($p in $AllowedPrefixes) { if ($rel -eq $p -or $rel.StartsWith($p)) { $ok = $true; break } }
    if (-not $ok) { return $null }
    $dest = [System.IO.Path]::GetFullPath((Join-Path $AgentHome ($rel -replace '/', '\')))
    $home2 = [System.IO.Path]::GetFullPath($AgentHome)
    if ($dest -ne $home2 -and -not $dest.StartsWith($home2 + [System.IO.Path]::DirectorySeparatorChar)) { return $null }
    return $dest
}

function Install-File([string]$src, [string]$dest) {
    $dir = Split-Path $dest -Parent
    if (-not (Test-Path $dir)) { New-Item -ItemType Directory -Path $dir -Force | Out-Null }
    $tmp = "$dest.gsupd.tmp"
    Copy-Item -LiteralPath $src -Destination $tmp -Force
    Move-Item -LiteralPath $tmp -Destination $dest -Force   # atomic replace
}

# ---- main ----
$work = Join-Path ([System.IO.Path]::GetTempPath()) ("gscontent." + [Guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $work -Force | Out-Null
$changed = 0
try {
    $mf = Join-Path $work 'manifest.json'; $sf = Join-Path $work 'manifest.json.sig'
    try { Get-Url 'manifest.json' $mf; Get-Url 'manifest.json.sig' $sf }
    catch { Write-Log "content channel unreachable ($($_.Exception.Message)) — retry next run"; return }

    if (-not (Test-Signature ([IO.File]::ReadAllBytes($mf)) ([IO.File]::ReadAllBytes($sf)))) {
        Write-Log 'ERROR: manifest signature INVALID — aborting (no files applied)'; exit 2
    }
    $manifest = Get-Content -Raw $mf | ConvertFrom-Json
    $newver = [string]$manifest.version
    $curver = if (Test-Path $State) { (Get-Content -Raw $State).Trim() } else { '' }
    if ($newver -and $newver -eq $curver) { return }   # already current
    Write-Log "content manifest $newver (channel $Channel/windows) — checking $($manifest.files.Count) files"

    foreach ($item in $manifest.files) {
        $rel = $item.path; $want = $item.sha256
        if (-not $rel -or -not $want) { continue }
        $dest = Resolve-SafeTarget $rel
        if (-not $dest) { Write-Log "REFUSED unsafe path in manifest: $rel"; continue }
        if ((Test-Path -LiteralPath $dest) -and ((Get-Sha256 $dest) -eq $want.ToLower())) { continue }
        $dl = Join-Path $work 'dl.bin'
        try { Get-Url "files/$rel" $dl } catch { Write-Log "  download failed for $rel"; continue }
        if ((Get-Sha256 $dl) -ne $want.ToLower()) { Write-Log "  HASH MISMATCH for $rel — skipped"; continue }
        Install-File $dl $dest
        $changed++
        Write-Log "  updated $rel ($($want.Substring(0,12)))"
    }
    if ($newver) {
        $sdir = Split-Path $State -Parent
        if (-not (Test-Path $sdir)) { New-Item -ItemType Directory -Path $sdir -Force | Out-Null }
        Set-Content -LiteralPath $State -Value $newver -NoNewline
    }
    Write-Log "content update complete: $changed file(s) changed, now at $newver"
} finally {
    Remove-Item $work -Recurse -Force -ErrorAction SilentlyContinue
}
