# HyperHide v5.2 — KUSER_SHARED_DATA lifecycle hardening

This tree is a copy of the preserved v5.1 NtQueryObject-fix source. The v5.1
tree and its runtime package are intentionally unchanged.

## Changed

- Save and restore the complete original KUSER_SHARED_DATA PTE.
- Replace the PTE atomically with `InterlockedExchange64`.
- Call the Windows 10 19045 `KeFlushEntireTb` export after installing and
  restoring the mapping.
- Restore and flush the original mapping before freeing the private page.
- Route driver-unload cleanup through the same unhook routine.
- Validate allocation/PTE setup and propagate hook failures.
- Reset the per-process clear state when the private mapping is removed.

## Intentionally unchanged

- `airhv.sys`, the plugin, profiles, offsets, and the v5.1 NtQueryObject fix.
- The existing `MmPfnDatabase` lookup and
  `0x200000000000000` flag operation. Its build-specific meaning needs more
  evidence before it is safe to change.

## Validation

- Release x64 built twice with Windows SDK/WDK 10.0.26100.0.
- The resulting SYS was byte-identical across both builds.
- NtQueryObject offline regression tests passed in both builds.
- The target Windows 10 19045 kernel was checked to export the parameterless
  `KeFlushEntireTb`, and the built driver imports it from `ntoskrnl.exe`.

This remains an experimental kernel build and requires bare-metal validation.
If it crashes, return to the archived v5.1-fixed package and retain the dump.
