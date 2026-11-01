HyperHide Win10 19045 x64 — v5.3 lifecycle-gate

Basis: v5.2 KUSD-lifecycle HyperHideDrv + korrigiertes LifecycleActive-Gate.
airhv.sys unverändert (v5/v5.2 known): C4F55AD0351B42CB6660538197AD3A7D9E7075676EC0920AC7A5900227E3DD6F

Korrektur gegenüber IDE-Commit 5975b64:
- Fix saß nur in vendor/HyperHide, nicht in v5.2 → hier in v5.2 portiert
- LifecycleActive erst nach vollständiger Init öffnen (nicht direkt nach test_vmcall)
- InterlockedExchange8 für Set/Clear
- Frühe DriverEntry-Fehlerpfade brauchen kein Clear mehr (Flag war nie 1)
- Commit-Text „open/close“ war irreführend: Gate ist Load/Unload, nicht IRP Create/Close

HyperHideDrv.sys SHA-256: FA6B467918574187A27E493AF1AD047DDBEE5EB10037062DC1FB3FDE4D65FD0B
HyperHideDrv.pdb SHA-256: 54E70E5AB56A46E506EB68E109B4F12DAC8EB134FB9DC16A26028B0B754073E0

Nicht auf dem Build-PC laden. Bench: install → start → Early-Attach / Unload-Race.
