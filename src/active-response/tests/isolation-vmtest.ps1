<#
  GuardsArm endpoint-isolation — LIVE behavioral test (Windows).
  Copyright (C) 2026, GuardSarm, Inc.

  WHAT THIS DOES
    Drives the real isolate.exe / repair-network.exe through a full behavioral
    cycle on a live host and asserts the SECURITY and SAFETY contract:
      - isolation actually cuts traffic to a non-allow-listed host,
      - the allow-listed manager stays reachable (endpoint stays managed),
      - repair-network.exe --force fully restores networking,
      - the fail-open watchdog (--auto) restores networking when the agent is
        gone / the dead-man timeout expires (the exact incident this design
        exists to prevent).

  !!! THIS CUTS THE HOST OFF THE NETWORK MID-RUN !!!
  Run it ONLY inside a THROWAWAY Windows VM with console (not RDP-only) access.
  It refuses to run unless:
      -IUnderstandThisWillCutNetworking     is passed, AND
      it detects a virtual machine  (override the VM check with -ForceNonVM,
                                     which you should basically never do).

  SAFETY NETS (belt-and-suspenders, so a crash can't strand the VM)
    - a one-shot scheduled task runs `repair-network.exe --force` at now+N min
      no matter what happens to this script;
    - every isolation step is wrapped so the finally block always repairs;
    - isolation timeouts in the test are tiny (seconds), not the 30-min default.

  USAGE (inside the VM, elevated PowerShell)
    .\isolation-vmtest.ps1 -IUnderstandThisWillCutNetworking `
        [-BinDir C:\Path\to\active-response\bin] `
        [-ManagerIp 10.0.0.5] [-ProbeHost 1.1.1.1] [-ProbePort 443] `
        [-DeadmanFuseMinutes 5]
#>
[CmdletBinding()]
param(
    [switch]$IUnderstandThisWillCutNetworking,
    [switch]$ForceNonVM,
    [string]$BinDir = "",
    [string]$ManagerIp = "",
    [string]$ProbeHost = "1.1.1.1",     # a host that must be BLOCKED while isolated
    [int]$ProbePort = 443,
    [int]$DeadmanFuseMinutes = 5        # hard external repair fuse (safety net)
)

$ErrorActionPreference = "Stop"
$script:pass = 0; $script:fail = 0; $script:results = @()

function Step($name, [bool]$ok, $detail="") {
    $tag = if ($ok) { "PASS" } else { "FAIL" }
    if ($ok) { $script:pass++ } else { $script:fail++ }
    $script:results += [pscustomobject]@{ Result=$tag; Check=$name; Detail=$detail }
    $color = if ($ok) { "Green" } else { "Red" }
    Write-Host ("  [{0}] {1}{2}" -f $tag, $name, $(if($detail){" — $detail"}else{""})) -ForegroundColor $color
}

function Test-Reachable([string]$h, [int]$p, [int]$timeoutMs=3000) {
    # TCP connect probe with a hard timeout; $true iff the 3-way handshake completes.
    try {
        $c = New-Object System.Net.Sockets.TcpClient
        $iar = $c.BeginConnect($h, $p, $null, $null)
        $okWait = $iar.AsyncWaitHandle.WaitOne($timeoutMs, $false)
        if ($okWait -and $c.Connected) { $c.EndConnect($iar); $c.Close(); return $true }
        $c.Close(); return $false
    } catch { return $false }
}

# ---- guardrails -------------------------------------------------------------
Write-Host "GuardsArm isolation behavioral test" -ForegroundColor Cyan

$id = [Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()
if (-not $id.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)) {
    throw "Must run elevated (WFP requires Administrator/SYSTEM)."
}
if (-not $IUnderstandThisWillCutNetworking) {
    throw "Refusing: pass -IUnderstandThisWillCutNetworking. This CUTS the host off the network. Use a throwaway VM."
}

$cs = Get-CimInstance Win32_ComputerSystem
$vmSig = "$($cs.Manufacturer) $($cs.Model)"
$isVM = $vmSig -match "VMware|VirtualBox|Virtual Machine|Hyper-V|KVM|QEMU|Xen|Parallels|innotek"
if (-not $isVM -and -not $ForceNonVM) {
    throw "Refusing: this does not look like a VM (`"$vmSig`"). Run in a throwaway VM, or pass -ForceNonVM if you REALLY mean it."
}
if (-not $isVM) { Write-Host "WARNING: -ForceNonVM set on a non-VM host ($vmSig). You accept the risk." -ForegroundColor Yellow }
Write-Host "Host: $vmSig  (VM detected: $isVM)"

# ---- locate the binaries ----------------------------------------------------
if (-not $BinDir) {
    foreach ($root in @("$env:ProgramFiles\gsmsec-agent","$env:ProgramFiles(x86)\gsmsec-agent","C:\Program Files\gsmsec-agent")) {
        if (Test-Path "$root\active-response\bin\isolate.exe") { $BinDir = "$root\active-response\bin"; break }
    }
}
$isolate = Join-Path $BinDir "isolate.exe"
$repair  = Join-Path $BinDir "repair-network.exe"
if (-not (Test-Path $isolate)) { throw "isolate.exe not found (looked in '$BinDir'). Pass -BinDir." }
if (-not (Test-Path $repair))  { throw "repair-network.exe not found (looked in '$BinDir'). Pass -BinDir." }
Write-Host "Binaries: $BinDir"

# ---- resolve the manager (allow-listed) address -----------------------------
if (-not $ManagerIp) {
    $agentRoot = Split-Path (Split-Path $BinDir -Parent) -Parent
    foreach ($cf in @("$agentRoot\etc\gsmsec.conf","$agentRoot\gsmsec.conf","$agentRoot\etc\ossec.conf")) {
        if (Test-Path $cf) {
            $m = Select-String -Path $cf -Pattern "<address>\s*([^<]+?)\s*</address>" | Select-Object -First 1
            if ($m) { $ManagerIp = $m.Matches[0].Groups[1].Value.Trim(); break }
        }
    }
}
if (-not $ManagerIp) { throw "Could not determine the manager IP. Pass -ManagerIp so isolation keeps the endpoint managed." }
Write-Host "Manager (allow-listed): $ManagerIp    Probe (must-block): $ProbeHost`:$ProbePort`n"

# Isolation AR messages (stdin JSON; command enable|disable; allow-list under guardsarm.active_response.isolation)
function Iso-Json([int]$timeout) {
@"
{"version":1,"command":"enable","parameters":{"program":"isolate"},"guardsarm":{"active_response":{"isolation":{"allow":["$ManagerIp"],"ports":[1514,1515,443],"allow_dns":true,"allow_dhcp":true,"timeout":$timeout}}}}
"@
}
$UnIsoJson = '{"version":1,"command":"disable","parameters":{"program":"isolate"},"guardsarm":{"active_response":{"isolation":{}}}}'

function Invoke-Isolate([string]$json) { $json | & $isolate; return $LASTEXITCODE }
function Repair([string]$mode) { & $repair $mode | Out-Null; return $LASTEXITCODE }
function Is-Isolated { & $repair "--status" | Out-Null; return ($LASTEXITCODE -eq 10) }  # --status exits 10 when isolated

# ---- external safety-net fuse: force-repair no matter what -------------------
$fuseTask = "GuardSarm VMTEST safety fuse"
try {
    $when = (Get-Date).AddMinutes($DeadmanFuseMinutes).ToString("HH:mm")
    schtasks /Create /TN "$fuseTask" /TR "`"$repair`" --force" /SC ONCE /ST $when /RU SYSTEM /RL HIGHEST /F | Out-Null
    Write-Host "Safety fuse armed: '$repair --force' will run at $when if this test ever hangs.`n" -ForegroundColor DarkGray
} catch { Write-Host "WARN: could not arm safety fuse task ($_)." -ForegroundColor Yellow }

try {
    # 0) preflight — non-destructive engine self-test MUST pass before we isolate
    Write-Host "Phase 0 — engine self-test (non-destructive)"
    Step "repair-network --selftest round-trips" ((Repair "--selftest") -eq 0)
    Step "host starts un-isolated" (-not (Is-Isolated))

    # 1) baseline connectivity
    Write-Host "Phase 1 — baseline connectivity"
    $baseProbe = Test-Reachable $ProbeHost $ProbePort
    $baseMgr   = Test-Reachable $ManagerIp 1514
    Step "probe host reachable before isolation" $baseProbe "$ProbeHost`:$ProbePort"
    if (-not $baseProbe) { throw "Baseline probe failed — pick a -ProbeHost/-ProbePort reachable from this VM." }

    # 2) ISOLATE
    Write-Host "Phase 2 — isolate"
    $rc = Invoke-Isolate (Iso-Json 120)
    Step "isolate.exe exit 0" ($rc -eq 0) "rc=$rc"
    Step "engine reports isolated" (Is-Isolated)

    # 3) containment contract: probe blocked, manager still reachable
    Write-Host "Phase 3 — containment contract"
    Start-Sleep -Seconds 2
    $isoProbe = Test-Reachable $ProbeHost $ProbePort 3000
    $isoMgr   = Test-Reachable $ManagerIp 1514 3000
    Step "non-allow-listed probe is BLOCKED while isolated" (-not $isoProbe) "$ProbeHost`:$ProbePort"
    Step "allow-listed manager STILL reachable while isolated" ($isoMgr) "$ManagerIp`:1514"

    # 4) recovery via --force
    Write-Host "Phase 4 — operator recovery (repair-network --force)"
    Step "repair-network --force exit 0" ((Repair "--force") -eq 0)
    Step "engine reports un-isolated after repair" (-not (Is-Isolated))
    Start-Sleep -Seconds 2
    Step "probe host reachable again after repair" (Test-Reachable $ProbeHost $ProbePort) "$ProbeHost`:$ProbePort"

    # 5) fail-open: agent gone while isolated -> --auto must restore
    Write-Host "Phase 5 — fail-open watchdog (agent crash simulation)"
    $svc = Get-Service -Name "GuardSarmSvc" -ErrorAction SilentlyContinue
    Invoke-Isolate (Iso-Json 120) | Out-Null
    $stoppedSvc = $false
    if ($svc -and $svc.Status -eq 'Running') { Stop-Service "GuardSarmSvc" -Force -ErrorAction SilentlyContinue; $stoppedSvc = $true }
    $autoRc = Repair "--auto"
    if ($svc) {
        Step "watchdog --auto fails OPEN when agent service stopped" (-not (Is-Isolated)) "auto rc=$autoRc"
    } else {
        # no service installed: --auto treats orphaned isolation as fail-open too
        Step "watchdog --auto fails OPEN for orphaned isolation" (-not (Is-Isolated)) "auto rc=$autoRc (no GuardSarmSvc)"
    }
    if ($stoppedSvc) { Start-Service "GuardSarmSvc" -ErrorAction SilentlyContinue }
    if (Is-Isolated) { Repair "--force" | Out-Null }

    # 6) dead-man timeout: isolate with a 1s fuse, wait, --auto must expire it
    Write-Host "Phase 6 — dead-man timeout"
    Invoke-Isolate (Iso-Json 1) | Out-Null
    Start-Sleep -Seconds 3
    $autoRc2 = Repair "--auto"
    Step "watchdog --auto expires isolation past dead-man timeout" (-not (Is-Isolated)) "auto rc=$autoRc2"
}
finally {
    # ---- guaranteed cleanup: never leave the VM isolated --------------------
    Write-Host "`nCleanup — forcing repair + clearing safety fuse"
    try { & $repair "--force" | Out-Null } catch {}
    try { Start-Service "GuardSarmSvc" -ErrorAction SilentlyContinue } catch {}
    try { schtasks /Delete /TN "$fuseTask" /F | Out-Null } catch {}
    $stillIso = try { Is-Isolated } catch { $true }
    if ($stillIso) {
        Write-Host "!! STILL ISOLATED after cleanup — run: `"$repair`" --force" -ForegroundColor Red
    } else {
        Write-Host "Network restored (host un-isolated)." -ForegroundColor Green
    }
}

# ---- summary ----------------------------------------------------------------
Write-Host "`n==== RESULTS ====" -ForegroundColor Cyan
$script:results | Format-Table -AutoSize
Write-Host ("{0} passed, {1} failed" -f $script:pass, $script:fail) -ForegroundColor $(if($script:fail){"Red"}else{"Green"})
exit $(if ($script:fail) { 1 } else { 0 })
