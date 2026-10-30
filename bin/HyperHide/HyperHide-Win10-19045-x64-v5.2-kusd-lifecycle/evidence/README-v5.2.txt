HyperHide v5.2 change evidence
Built: 2026-10-30

The other files in this evidence directory document the v5-to-v5.1
NtQueryObject fix inherited by v5.2.

v5.2 changes only the HyperHideDrv KUSER_SHARED_DATA lifecycle:
- complete original PTE is saved and restored
- PTE replacement uses InterlockedExchange64
- KeFlushEntireTb is called after install and restore
- the original mapping is restored before the private page is freed
- unload uses the common unhook routine
- allocation/PTE errors are returned instead of leaving a false active state

KeFlushEntireTb was verified as an exported, parameterless function in the
Windows 10 19045 ntkrnlmp.exe used for dump analysis. The resulting driver also
imports KeFlushEntireTb from ntoskrnl.exe.

Intentionally unchanged:
- airhv.sys
- HyperHide plugin and configuration
- NtQueryObject v5.1 fix
- existing MmPfnDatabase lookup and the 0x200000000000000 flag operation

Offline validation:
- Release x64 build succeeded twice
- HyperHideDrv.sys was byte-identical across both builds
- NtQueryObject safety regression tests passed in both builds

HyperHideDrv.sys SHA256:
2798C37855CA06B6FCA64B32E8794ED0E4D02F16793D0BA94369DB19EEA6E681
