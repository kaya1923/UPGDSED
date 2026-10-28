# HyperHide crash analysis (2026-10-28)

## Evidence

- Dump: `102826-5968-01.dmp`
- SHA-256: `BC84B8D96654074133A250EA3FFFC22D11217D0352D1B52DFBBD573CE3D69176`
- Bugcheck: `MEMORY_MANAGEMENT (0x1A)`, subtype `0x3600`
- Crash CPU: 2
- Uptime: 2 minutes 37 seconds
- Loaded HyperHide component: `airhv.sys`, timestamp `2026-10-28 03:09:02`
- `HyperHideDrv.sys` was not loaded.

The active stack was in the Windows memory manager while it was checking page
tables during an unrelated NDIS request. This is where the corruption was
detected, not where it originated.

## Root-cause finding

`airhv` assigns each logical processor a distinct VPID:

```cpp
KeGetCurrentProcessorNumberEx(NULL) + 1
```

The original VM-exit paths nevertheless passed the constant VPID `1` to every
INVVPID operation. On this four-thread system, CPUs 1-3 therefore invalidated
the translations of CPU 0 instead of their own. Stale translations can direct
memory writes to obsolete physical pages and are consistent with the corrupted
page-table entries reported by bugcheck `0x1A/0x3600` on CPU 2.

## Correction

- Every VPID-targeted invalidator now reads the active VPID directly from
  `CONTROL_VIRTUAL_PROCESSOR_IDENTIFIER` in the current VMCS.
- The bundled driver is built with `_MINIMAL` (`Release_Minimal`) so HyperHide
  does not enable unrelated CR3, INVLPG, timing, descriptor-table, debug-register,
  I/O, and MSR interception paths.
- The start workflow supports `-AirhvOnly` for staged testing.
- Installation and start scripts reject any `airhv.sys` whose SHA-256 is not:
  `C4F55AD0351B42CB6660538197AD3A7D9E7075676EC0920AC7A5900227E3DD6F`.

## Limitations

The minidump proves which component was active and documents the resulting
page-table corruption. It does not contain the earlier write that damaged the
entry. The VPID defect is a source-level causal match, but only a controlled
runtime test can establish that no additional hardware-specific defects remain.
