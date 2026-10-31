# HyperHide v5.1 — NtQueryObject-Sicherheit und Regressionstests

Stand: 29.10.2026, HP-Bench. Dieses Dokument ergänzt
[HANDOFF_NEXTSTEPS_20261029.md](HANDOFF_NEXTSTEPS_20261029.md) und
[HANDOFF_HYPERHIDE_V5_V6.md](HANDOFF_HYPERHIDE_V5_V6.md).

## Status

- **NtQueryObjectSafety-Regressionstests: BESTANDEN (29.10.2026).**
  Kompiliert mit MinGW-Builds GCC 16.2.0 (`-std=c++17 -Wall -Wextra -O2`)
  ohne Warnungen (Exit 0) und erfolgreich ausgeführt
  (`NtQueryObject safety regression tests passed`, Exit 0).
- Der v5.1-Treiber (v5.1-Hook) ist code-complete und offline verifiziert; die
  native HP-Bench-Validierung steht noch aus (siehe HANDOFF, "Nächster
  HP-Bench-Schritt").

## Hintergrund (Warum v5.1)

v5 stürzt (BSOD `0x3B`) in `nt!RtlEqualUnicodeString`, aufgerufen aus
`HyperHideDrv!HookedNtQueryObject` (`HyperHideDrv+0x3B26`), wenn
`ReturnLength[4]` und `ObjectInformation` im Benutzeradressraum überlappen.
Direkt davor war `STATUS_OBJECT_NAME_EXISTS` zurückgekommen — daher ist
`ObjectAttributes.Length` ein Benutzerwert und darf NICHT mit `ULONG_MAX`
gesättigt werden.

Der v5.1-Hook macht `NtQueryObject` sicher, ohne `RtlEqualUnicodeString` zu
hooken:

- Bei jeder Überlappung von `ReturnLength[4]` und `ObjectInformation` wird die
  Nachbearbeitung übersprungen; Originalstatus und Originalausgabe bleiben.
- Der Hook liest oder restauriert `ReturnLength` selbst nicht mehr.
- Class 2 kopiert den nativen x64-Descriptor und Namen unter SEH in lokale
  Kernelvariablen. Kein Benutzerpointer geht an `RtlEqualUnicodeString`.
- Class 3 wird bis maximal 1 MiB in NonPagedPoolNx gespiegelt; Anzahl,
  Descriptoren, UTF-16-Längen, Pointerkette, Addition und Alignment werden vor
  der einzigen möglichen Benutzerschreiboperation vollständig validiert.
- Alle verbleibenden Benutzerzugriffe sind mit `ProbeForRead/ProbeForWrite`
  und SEH abgesichert. Unsichere Fälle behalten den Originalreturn.
- Zähler werden sättigend statt mit Unterlauf reduziert.

## Quellstand

- `vendor/HyperHide-v5.1/` — aus dem verifizierten v5-Snapshot erzeugt.
- `vendor/HyperHide-v5.1/HyperHideDrv/NtQueryObjectSafety.h` — die
  Nachbearbeitung/Helfer: `CheckedRangeEnd`, `RangeContains`,
  `RangesOverlap`, `AlignUp`, `ValidTypeName`, `CanPostProcess`.
- `vendor/HyperHide-v5.1/tests/ntqueryobject_safety_tests.cpp` — die
  Regressionstests.
- Quelldiff: [HyperHide-v5-to-v5.1-ntqueryobject.patch](HyperHide-v5-to-v5.1-ntqueryobject.patch).

## Regressionstests (ausgeführt 29.10.2026)

Quelldatei: `vendor/HyperHide-v5.1/tests/ntqueryobject_safety_tests.cpp`
Deckt ab: `RangesOverlap`, `RangeContains`, `AlignUp`, `ValidTypeName`,
`CanPostProcess` — inklusive der beobachteten Crash-Überlappung
(`ObjectInformation=0x0528FA70`, `ReturnLength=0x0528FA78`).

Testfälle (aus der Quelldatei):

1. `RangesOverlap(0x0528FA70, 0x0528FA78)` — die Dump-Überlappung wird erkannt;
   angrenzende Bereiche (`0x0528FA6F`/`0x0528FA70`) überlappen nicht.
2. `RangeContains` — ein 4-Byte-Range enthält kein Byte außerhalb.
3. `AlignUp` — 16 wird korrekt zu 16/24/32/48/64/80/96/112/128 ausgerichtet.
4. `ValidTypeName` — NULL, leere und überlange Namen werden verworfen.
5. `CanPostProcess` — Überlappung und gültiger Typ werden beide erkannt.

Ergebnis (29.10.2026, MinGW-Builds GCC 16.2.0, x86_64-posix-seh):

- Kompilierung: Exit 0, keine Warnungen (`-std=c++17 -Wall -Wextra -O2`).
- Ausführung: Exit 0, Ausgabe `NtQueryObject safety regression tests passed`.

Reproduktionsbefehl (PowerShell, aus dem Projektordner):

```powershell
$gxx = 'C:\Users\k\Desktop\_mingw7z\ext2\mingw64\bin\g++.exe'
& $gxx -std=c++17 -Wall -Wextra -O2 `
  vendor\HyperHide-v5.1\tests\ntqueryobject_safety_tests.cpp `
  -o ntqueryobject_safety_tests.exe
.\ntqueryobject_safety_tests.exe
# -> NtQueryObject safety regression tests passed
```

## Abgrenzung / Offen

- Die Regressionstests verifizieren die v5.1-Helfer/Nachbearbeitungslogik
  offline. Sie beweisen NICHT die native Stabilität auf der HP-Bench — der
  Treiber ist dort noch nicht gelaufen (siehe HANDOFF, "Nächster
  HP-Bench-Schritt").
- Airhv-VM-Erkennungsfix: bewusst noch nicht geändert; benötigt die
  Drei-Zustands-CPUID-Messwerte der HP-Bench.

## Verwandte Dokumente

- [HANDOFF_NEXTSTEPS_20261029.md](HANDOFF_NEXTSTEPS_20261029.md)
- [HANDOFF_HYPERHIDE_V5_V6.md](HANDOFF_HYPERHIDE_V5_V6.md)
- [../nextsteps.md](../nextsteps.md)
