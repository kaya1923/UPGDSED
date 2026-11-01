HyperHide Win10 19045 x64 — v5.5 vmcall #UD guard

Context (dump 103126): 0x1E / 0xC000001D at vmcall while airhv VMXOFF.
Dump was BEFORE v5.4 build; v5.4 package also had wrong expectedHideHash
so install.ps1 would reject the new SYS.

Changes vs v5.4
- SafeVmCall/SafeVmCallEx: SEH catches STATUS_INVALID_OPCODE (#UD), marks OFFLINE
- stop.ps1 waits until HyperHideDrv STOPPED before stopping airhv
- install/start.ps1 stamped with this HyperHideDrv hash

HyperHideDrv.sys SHA-256: DA9FA1812099310EFD30D3F05E353030EFF33B62F0CA100852E222FC7CAC9349
airhv.sys SHA-256: C4F55AD0351B42CB6660538197AD3A7D9E7075676EC0920AC7A5900227E3DD6F (unchanged)

Bench: install.ps1 → start.ps1 → 20+ stop/start. Do not load on build PC.
