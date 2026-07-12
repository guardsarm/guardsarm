# Endpoint-isolation tests

Two levels, matching the two risk tiers of the WFP isolation engine.

## 1. Non-destructive engine self-test — safe on ANY host (incl. production)

`repair-network.exe --selftest` exercises the full WFP lifecycle
(create provider + sub-layer → add PERMIT-only filters → enumerate → delete →
confirm cleanup) inside a transaction. It installs **no block-all filter**, so
it can never isolate anything. Requires Administrator/SYSTEM.

```powershell
& "C:\Program Files\gsmsec-agent\active-response\bin\repair-network.exe" --selftest
# exit 0 = engine round-trips correctly; 1 = engine error
```

Run this on any endpoint before trusting isolation there.

## 2. Live behavioral test — THROWAWAY VM ONLY

`isolation-vmtest.ps1` drives the real `isolate.exe` / `repair-network.exe`
through a full cycle and asserts the security + safety contract:

| Phase | Asserts |
|---|---|
| 0 self-test | engine round-trips; host starts un-isolated |
| 1 baseline | probe host + manager reachable |
| 2 isolate | `isolate.exe` exit 0; engine reports isolated |
| 3 containment | non-allow-listed probe **blocked**; allow-listed manager **still reachable** |
| 4 recovery | `--force` restores networking; probe reachable again |
| 5 fail-open | agent stopped while isolated → `--auto` restores networking |
| 6 dead-man | isolation past its timeout → `--auto` expires it |

**It cuts the host off the network mid-run.** It refuses to start unless
`-IUnderstandThisWillCutNetworking` is passed *and* it detects a VM, requires
elevation, arms an external `schtasks` fuse that force-repairs after
`-DeadmanFuseMinutes`, and always force-repairs in a `finally` block — so a
crash cannot strand the VM.

```powershell
# elevated PowerShell, inside a throwaway VM with console access:
.\isolation-vmtest.ps1 -IUnderstandThisWillCutNetworking `
    -ManagerIp 10.0.0.5 -ProbeHost 1.1.1.1 -ProbePort 443
```

`-BinDir` overrides binary discovery; `-ManagerIp` is auto-read from
`etc/gsmsec.conf` if omitted. Never run on a machine you cannot afford to lose
network on. Do **not** run on the developer workstation (that is the incident
this whole design exists to prevent).
