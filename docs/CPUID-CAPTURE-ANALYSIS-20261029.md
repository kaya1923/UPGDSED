# CPUID three-state capture analysis (HP Bench)

Stand: 29.10.2026. Quelle:
`mmf-bin-bench-min/.../cpuid-bench-20261029/captures/` auf
`KAYAS-HP-DEBUG`, Windows Build 19045. Gleiche Diagnosebinärdateien,
x64-Auswertung.

## Ergebnis in einem Satz

v5-airhv leakt Hypervisor-Bit und Vendorstring; v6-airhv matcht die
gemessenen CPUID-Register von Bare Metal. Der verbleibende, in allen
airhv-Läufen sichtbare Unterschied ist die CPUID-Latenz (ca. 3–6×).

## Messwerte (cpuid-x64)

| Zustand | HV present | leaf1 ECX | vendor 0x40000000 | leaf0 median cycles |
|---------|------------|-----------|-------------------|---------------------|
| drivers-off | 0 | 7FFAFBFF | invalid-leaf mirror (`00000007` / `00000340`) | 144 |
| v5-airhv-only | 1 | FFFAFBFF (Bit 31) | `airh` / `v` (`40000001`, `68726961`, `00007600`) | 756 |
| v6-airhv-only | 0 | 7FFAFBFF | identisch zu drivers-off | 938 |

Register-Diff v6 vs drivers-off: nur `leaf 1` EBX (`01100800` vs
`02100800`) — typische APIC-/Topology-Varianz zwischen Kernen, kein
HV-Leak. Alle übrigen erfassten Leaves inkl. `0x40000000`..`0x4FFFFFFF`
sind byteidentisch zum Bare-Metal-Lauf.

## Schlussfolgerungen

1. Ein weiterer Register-Spoof von leaf `0x40000000` / Bit 31 ist für
   v6 nicht begründet; die Captures zeigen dort schon natives Verhalten.
2. Die Flex-VM-Meldung unter airhv-only (auch ohne HyperHideDrv) kann
   nicht über diese Registerdifferenz erklärt werden. Kandidaten:
   CPUID-Exit-Timing (hier gemessen), andere VM-Exit-Nebenwirkungen,
   oder ein nicht-CPUID-Sensor in Flex.
3. CPUID-Exits sind unter Intel VMX non-root unbedingt; sie lassen sich
   nicht per Primary-Control abschalten. Ein Timing-Ausgleich wäre eine
   eigene, isolierte airhv-Variante (z. B. TSC-Offset-Experimente) und
   darf nicht mit dem NtQueryObject-Crashfix vermischt werden.
4. Nächster Offline-Schritt: Flex-7.59.1 auf den konkreten Sensor
   eingrenzen (String/Xref/CPUID-Nutzung). Nächster Runtime-Schritt auf
   der HP-Bench: v5.1-NtQueryObject-Paket laden und Early-Attach testen;
   VM-Dialog-Erwartung bei airhv unverändert, bis ein Timing-/Sensor-Fix
   vorliegt.

## Abgrenzung

Diese Analyse ändert keine Treiber. Sie ersetzt keinen erfolgreichen
Flex-Starttest auf der Bench.
