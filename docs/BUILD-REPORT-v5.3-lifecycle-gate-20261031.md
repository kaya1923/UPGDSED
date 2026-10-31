# Review + build: LifecycleActive gate → v5.3

Date: 2026-10-31 (build PC). Driver not loaded here.

## Review of commit `5975b64` (other IDE)

Idea is sound: gate `hypervisor_visible` / `dump_pool_manager` so late IOCTLs
cannot VMCALL during unload. Hooks/unhook/`test_vmcall` correctly left ungated.

Problems found:

1. **Wrong tree for bench packages** — change only in `vendor/HyperHide/`,
   not in `vendor/HyperHide-v5.2/` (what v5.2 ZIPs are built from).
2. **Gate opened too early** — `LifecycleActive=1` right after `test_vmcall`,
   before offsets/SSDT/hooks/device. Intermediate DriverEntry failures left
   the flag set (harmless on failed load, but messy); IOCTL path could in
   theory race a partial init once the device exists.
3. **Commit wording** claimed “open/close”; `DrvCreate`/`DrvClose` never
   touched the flag (load/unload only).
4. Plain stores instead of an interlocked publish/clear.

## Correction (this build)

- Ported into **v5.2** source tree and mirrored into `vendor/HyperHide/`.
- Open gate **only after** device + symbolic link are created (full success).
- Clear with `InterlockedExchange8` at the start of `DrvUnload`.
- Offline NtQueryObject safety tests still pass.

## Artifact

- ZIP: `69B7679309F336FE1B35B013CAFFFBC87927067BD9B0A73C9F7BD07553B8D5DD`
  (`bin/HyperHide/HyperHide-Win10-19045-x64-v5.3-lifecycle-gate.zip`)
- `HyperHideDrv.sys` SHA-256
  `FA6B467918570187A27E493AF1AD047DDBEE5EB10037062DC1FB3FDE4D65FD0B`
- `airhv.sys` unchanged `C4F55AD0…` (isolate HyperHideDrv variable)
- For CPUID timing use the separate `v5.2-cpuid-cache` airhv package.
