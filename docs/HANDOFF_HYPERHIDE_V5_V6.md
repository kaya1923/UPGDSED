# Handoff: UPGDSED + HyperHide/airhv v5 and v6

Date: 2026-10-29  
Target used for runtime checks: HP EliteBook 840 G1, Intel Core i5-4300U,
Windows 10 22H2 build 19045, kernel `10.0.19041.7802`.

## Status at handoff

This repository contains an experimental Windows kernel/hypervisor test stack.
It is not a production-ready security bypass. Use only on a recoverable test
machine with a normal boot entry available.

### v5: known-good startup baseline

Confirmed on the target machine:

- BIOS VT-x/EPT enabled.
- Microsoft Hyper-V and VBS disabled.
- `airhv` reached `SERVICE_RUNNING`.
- `HyperHideDrv` reached `SERVICE_RUNNING`.
- The UPGDSED boot entry loads the patched kernel as `ntkrnlmp.exe`.
- HyperHide resolves that kernel successfully despite the filename alias.

This confirms driver initialization. It does **not** prove that every debugger
hook is stable. Sporadic BSODs have subsequently been reported while actively
debugging; a new crash dump is still required.

Artifact:

`bin/HyperHide/HyperHide-Win10-19045-x64-v5-kernel-alias.zip`

SHA-256:

`96831611B9D87B9222D0C2644FFE9155C162AD62D485667694EF8877602B3F7F`

Important contained binaries:

- `airhv.sys`: `C4F55AD0351B42CB6660538197AD3A7D9E7075676EC0920AC7A5900227E3DD6F`
- `HyperHideDrv.sys`: `C9DC14B85020B32E4D08DC22B66B6FC7F5E247FDC73CD062B0D7352B57AB8D57`

An immutable duplicate and the v5 source snapshot are under
`bin/HyperHide/known-good/`.

### v6: CPUID-stealth test variant

v6 retains the v5 `HyperHideDrv.sys` byte-for-byte and changes only airhv's
initial CPUID visibility plus the x32dbg/x64dbg plugin visibility lifecycle.

Changes:

- airhv initializes `g_vmm_context->hv_presence` to `false`.
- `CPUID(1).ECX[31]` is therefore cleared by the existing VM-exit handler.
- Hypervisor leaves use airhv's existing hidden/bare-metal fallback instead of
  returning the airhv vendor interface.
- Closing the plugin no longer calls `SetHyperVisorVisibility(TRUE)`.
- The `Hypervisor not visible` menu item starts checked and stays synchronized.
- Both `HyperHide.dp32` and `HyperHide.dp64` are included.
- `cpuid_verify.exe` reports CPUID leaf values and timing percentiles.

Artifact:

`bin/HyperHide/HyperHide-Win10-19045-x64-v6-cpuid-stealth.zip`

SHA-256:

`462136E19C1E4FB217C9EE2F34D167B04D7589CC19E83EA7F94065362832D519`

Important contained binaries:

- `airhv.sys`: `A4EFF0AFA2E161A1ABE1F57FA7034C98229FBC82D11BF96DF70EE113F8681A3B`
- `HyperHideDrv.sys`: `C9DC14B85020B32E4D08DC22B66B6FC7F5E247FDC73CD062B0D7352B57AB8D57`
- `HyperHide.dp64`: `B6478D9EDB22ADE0173A3A9BD5E78C09FDA2C095CAEAEC748AA14D0F149FC172`
- `HyperHide.dp32`: `19EE015615961C8FB9517562D43BE8749475DC6BB109A4A70518D630E7952F4E`
- `cpuid_verify.exe`: `7E9A54743D7968AA363C52D2FDED83998751505014F5612BE4F312459832E1A4`

v6 was built and statically validated. Its CPUID behavior still needs to be
recorded on the HP target before application-specific testing.

## Source layout

- `src/`: UPGDSED source, including Windows 10 19041-19045 scanner support and
  the interactive dry-run path.
- `vendor/HyperHide/`: clean vendored HyperHide source with the v5/v6 changes.
- `vendor/HyperHide/airhv/`: airhv source at upstream submodule revision plus
  the VPID invalidation and CPUID-default changes.
- `bin/HyperHide/`: versioned runtime bundles.
- `symbols/`: matching Microsoft public PDBs used for offset/crash analysis.
- `HYPERHIDE_CRASH_ANALYSIS.md`: analysis of the original airhv-only crash.

Upstream identities used during development:

- HyperHide: `Air14/HyperHide`, revision
  `cbbf364c8358ce479011139ef2ae1f72d5bceeec`.
- airhv submodule: revision beginning `d2b4a6c`.

## Relevant fixes

### UPGDSED

- Added scanner support for Windows builds 19041 through 19045.
- Corrected a `PatternSize` assignment typo for newer scanner handling.
- Added interactive dry-run output with untouched files in `dryrun/ori` and
  modified copies in `dryrun/mod`; dry-run does not alter BCD or System32.
- Improved the BadRk test controller's console reporting.

### airhv stability

The first airhv build crashed with `MEMORY_MANAGEMENT (0x1A/0x3600)`. The source
used the logical processor number as VPID at VMCS setup but invalidated VPID 1
unconditionally. `invalidators.cpp` now reads the current VPID from the VMCS for
every INVVPID descriptor. airhv is built as `Release_Minimal`.

### HyperHideDrv compatibility

- Modern-WDK compatibility guard in `Ntenums.h`.
- Windows 10 `KiDispatchException` direct-prologue signature for kernel
  `10.0.19041.7802`.
- Stage-specific DriverEntry status codes for diagnostics.
- Kernel lookup resolves the first `SystemModuleInformation` entry when the
  caller asks for `ntoskrnl.exe`, supporting UPGDSED's `ntkrnlmp.exe` alias.

## Verified kernel offsets

The following were verified against the matching Microsoft PDB identified by
GUID/age `91A0697CBDF1F6C1B8D282E109E671E61`:

| Structure field | Offset |
| --- | ---: |
| `_KTHREAD.MiscFlags` | `0x74` |
| `_ETHREAD.CrossThreadFlags` | `0x510` |
| `_ETHREAD.PicoContext` | `0x5E0` |
| `_EPROCESS.Flags2` | `0x460` |
| `_EPROCESS.SeAuditProcessCreationInfo` | `0x5C0` |

There is currently no evidence that these offsets cause the intermittent
debugging BSOD. Higher-risk paths include KUserSharedData page substitution,
KiDispatchException/EPT hooks, context/debug-register hooks, and object-lifetime
races. Do not guess: collect and analyze the new minidump.

## Safe v5 restore/start

From an elevated PowerShell in the extracted v5 directory:

```powershell
powershell -ExecutionPolicy Bypass -File .\preflight.ps1
powershell -ExecutionPolicy Bypass -File .\install.ps1
powershell -ExecutionPolicy Bypass -File .\start.ps1
```

Expected state:

```powershell
sc.exe query airhv
sc.exe query HyperHideDrv
```

Both should show `STATE : 4 RUNNING`. Stop after closing all debugger instances:

```powershell
powershell -ExecutionPolicy Bypass -File .\stop.ps1
```

Services are `DEMAND_START`; the bundle does not configure automatic startup.

## v6 A/B test

Stop v5 first, then record the native baseline before starting v6:

```powershell
.\cpuid_verify.exe | Tee-Object .\cpuid-before.txt
powershell -ExecutionPolicy Bypass -File .\install.ps1
powershell -ExecutionPolicy Bypass -File .\start.ps1 -AirhvOnly
.\cpuid_verify.exe | Tee-Object .\cpuid-after-airhv.txt
```

Expected after v6 airhv starts:

- `Hypervisor bit: 0`.
- No `airhv`, `Microsoft Hv`, `VMware`, `KVM`, or `VBox` vendor string.
- Leaf values should be compared with the stopped-airhv baseline.

Then stop airhv, start the complete stack, and repeat the capture before and
after opening/closing the debugger. Leave `Hypervisor not visible` checked.

## Open issue: intermittent debugging BSOD

The only dump currently analyzed predates v5 and contained airhv without
HyperHideDrv. It cannot explain the later debugging crash. The next handoff
step is:

1. Stop active HyperHide testing until a new dump is available.
2. Copy the newest `C:\Windows\Minidump\*.dmp` from the HP after the crash.
3. Record selected profile, attach/create mode, last debugger action and whether
   hardware breakpoints were active.
4. Analyze bugcheck, faulting instruction, stack, active hooks and driver hashes.
5. Only then change or disable the implicated hook.

Crash dumps are intentionally excluded from Git because they may contain
machine memory and user data. The four matching public Microsoft PDBs are
included under `symbols/` (about 12.4 MB) with hashes and cache identities.

## Build environment used

- Visual Studio Community 18
- MSVC `14.51.36231`
- Windows SDK/WDK `10.0.26100.0`
- Windows 10 SDK `10.0.19041.0` also installed
- x64 HyperHideDrv and airhv linked as native kernel drivers
- airhv configuration: `Release_Minimal`
- plugins built for x64 (`.dp64`) and Win32 (`.dp32`)

Because VS/WDK integration did not recognize the older project configuration,
the final driver variants were built with explicit compiler, include, library
and linker paths. Preserve the release hashes above when reproducing them.

