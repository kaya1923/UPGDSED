# v5.5 — vmcall #UD guard (follow-up to dump 103126)

Date: 2026-10-31 (build PC). Driver not loaded here.

## Dump 103126

- Bugcheck `0x1E`, exception `0xC000001D` (#UD) at `HyperHideDrv` `vmcall`
- Process `pwsh.exe`, DPC path — HyperHide issued VMCALL while airhv was
  not in VMX operation (VMXOFF / reload churn; 4 unloaded HyperHide images).
- Timestamp **15:36** — **before** the v5.4 build (~16:51). So this dump does
  **not** invalidate the v5.4 drain logic; it motivates a harder guard.

## Extra packaging bug found

v5.4 ZIP still had `expectedHideHash = 2798C378…` (old v5.2). `install.ps1`
would refuse the new `FE41B4D8…` SYS. v5.5 stamps the real driver hash.

## Fix

- `SafeVmCall` / `SafeVmCallEx`: `__try/__except` on `0xC000001D`, mark
  `VmcallState=OFFLINE` (DISPATCH_LEVEL-safe; no device lookups in DPC).
- `stop.ps1`: wait until HyperHideDrv is `STOPPED` before `sc stop airhv`.
- Keep v5.4 lifecycle drain/unhook (no HyperHide-side `vmoff`).

## Artifact

- ZIP SHA-256: `740D9D200C599EC39BCE7D7AF43A9111047F01A14070A5C4889E2E57A3BF0F27`
- `HyperHideDrv.sys` SHA-256: `DA9FA1812099310EFD30D3F05E353030EFF33B62F0CA100852E222FC7CAC9349`
- `airhv.sys` unchanged `C4F55AD0…`
- NtQueryObject offline tests: passed

## Bench

1. Pull; use `packages-20261029/HyperHide-Win10-19045-x64-v5.5-vmcall-ud-guard.zip`
2. `install.ps1` (must accept hash) → `start.ps1` → 20+ `stop.ps1`/`start.ps1`
3. Do not stop airhv before HyperHideDrv
