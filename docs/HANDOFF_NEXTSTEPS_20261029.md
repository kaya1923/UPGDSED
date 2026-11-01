# Handoff: v5.1 NtQueryObject fix and CPUID diagnostics

Stand: 29.10.2026

> Nachtrag 30.10.2026: v5.1 bleibt der dokumentierte Rollback-Zustand, die
> aktuelle Runtime ist jetzt v5.2 — siehe
> [HANDOFF_V5.2_KUSER_LIFECYCLE_20261030.md](HANDOFF_V5.2_KUSER_LIFECYCLE_20261030.md)
> (v5.2-Kompletierung, verifizierte Artefakt-Hashes, Repo-/Remote-Stand,
> verbleibende Schritte).

## Ergebnis

Prompt 1 aus `nextsteps.md` ist als isolierte v5.1-Variante umgesetzt. Das
bekannte v5 und v6 sowie `airhv.sys`, Plugin, Offsets, Kernelalias,
VPID-Invalidierung und KiDispatchException-Signatur wurden nicht verändert.

Prompt 2 ist bis zum geforderten reproduzierbaren Diagnosepaket umgesetzt.
Eine airhv-Verhaltensänderung ist noch nicht enthalten: Erst die Messwerte der
HP-Bench dürfen bestimmen, ob und wie ein CPUID-VM-Exit minimal geändert wird.

## Beobachtungen, Schlussfolgerungen und offene Punkte

Beobachtet in beiden autorisierten Dumps:

- Bugcheck `0x3B`, Exception `0xC0000005` in `nt!RtlEqualUnicodeString+0x43`.
- Aufrufer `HyperHideDrv+0x3B26`, entsprechend `HookedNtQueryObject`.
- Information class 2, Originalstatus erfolgreich.
- `ObjectInformation=0x0528FA70`, `ReturnLength=0x0528FA78`.
- Auf x64 liegt `UNICODE_STRING.Buffer` bei Offset `+8`; die beiden
  Ausgabebereiche überlappten daher exakt.

Begründete Schlussfolgerung: Der alte Hook vertraute einem Benutzerpuffer nach
dem Originalaufruf, übergab dessen Descriptor direkt an
`RtlEqualUnicodeString` und dereferenzierte `ReturnLength` ungeschützt. Die
beobachtete Überlappung konnte den Stringzeiger auf `0x80` verfälschen.

Offen: Minidumps beweisen nicht den exakten Schreibzeitpunkt. Der neue Code
verhindert den beobachteten Pfad mechanisch, ist aber noch nicht nativ auf der
HP-Bench gelaufen. Erfolgreicher Build oder `SERVICE_RUNNING` beweisen weder
Stabilität beim Debuggen noch Anwendungskompatibilität.

## v5.1-Änderung

Quelle: `vendor/HyperHide-v5.1/`, aus dem verifizierten v5-Snapshot erzeugt.

- Bei jeder Überlappung von `ReturnLength[4]` und `ObjectInformation` wird die
  Nachbearbeitung übersprungen; Originalstatus und Originalausgabe bleiben.
- Der Hook liest oder restauriert `ReturnLength` selbst nicht mehr.
- Class 2 kopiert den nativen x64-Descriptor und Namen unter SEH in lokale
  Kernelvariablen. Kein Benutzerpointer geht an `RtlEqualUnicodeString`.
- Class 3 wird bis maximal 1 MiB in NonPagedPoolNx gespiegelt. Anzahl,
  Descriptoren, UTF-16-Längen, Pointerketten, Addition und Alignment werden vor
  der einzigen möglichen Benutzerschreiboperation vollständig validiert.
- Alle verbleibenden Benutzerzugriffe sind mit `ProbeForRead/ProbeForWrite`
  und SEH abgesichert. Unsichere Fälle behalten den Originalreturn.
- Zähler werden sättigend statt mit Unterlauf reduziert.

Build:

- VS Community 2026 18.11.2, MSVC 14.51.36231, SDK/WDK 10.0.26100.0.
- Release x64, Native subsystem, `DriverEntry`, vollständige PDB.
- Das alte WDK-MSBuild-Platformtoolset ist in VS 2026 nicht registriert
  (`MSB8020`); deshalb existiert ein explizites Buildskript:
  `vendor/HyperHide-v5.1/build-release-x64.ps1`.
- Zwei saubere aufeinanderfolgende Builds erzeugten dieselbe SYS. Die interne
  Voll-PDB-Datei ist linkerbedingt nicht byteidentisch; zum Paket gehört exakt
  die zur ausgelieferten SYS erzeugte PDB. Die in der SYS gespeicherte RSDS-ID
  ist `{D4F0DF80-6019-8C8A-5B3A-EB697E4873A3}`, Age 1.

## Ausgeführte Offlineprüfungen

- Release-x64-Treiber vollständig kompiliert und gelinkt.
- Native PE-Header geprüft: Machine x64, Subsystem Native, Entry `DriverEntry`.
- Shared Regressionen bestanden: Dump-Überlappung bei `+8`, NULL/getrenntes
  ReturnLength, angrenzende Bereiche, Überläufe, kurze/externe/geänderte
  Stringbereiche, native x64-Layoutannahme und Class-3-Alignment.
- Alle unveränderten v5-Quelldateien byteweise gegen den Snapshot geprüft:
  keine unerwartete Differenz.
- Unveränderte `airhv.sys` im v5.1-Paket:
  `C4F55AD0351B42CB6660538197AD3A7D9E7075676EC0920AC7A5900227E3DD6F`.
- Paket-PowerShell syntaktisch geprüft; ZIP zurückentpackt und Manifestdateien
  byteweise per SHA-256 geprüft.
- Kein Treiber geladen, kein Dienst geändert, kein Installer ausgeführt und
  keine BCD-Einstellung auf dem Buildrechner verändert.

Compilerwarnungen aus dem unveränderten v5-Bestand (C4312 und veraltetes
`ExAllocatePoolWithTag`) sind im vollständigen Buildlog erhalten. Der neue Hook
verwendet den alten Pool-API bewusst weiter, um keine zusätzliche Import- oder
OS-Kompatibilitätsänderung mit dem Crashfix zu vermischen.

## Artefakte

v5.1 Runtime und Symbole:

- `bin/HyperHide/HyperHide-Win10-19045-x64-v5.1-ntqueryobject-fix.zip`
- SHA-256 `3580804C36F27504585A670236350EC37B271E61B8A70B34D2FA4205736C95E4`
- `HyperHideDrv.sys` SHA-256
  `90974C370093984DCCEA35B6A9EBF3C54F961B35FF1BCEE28E657D4AABAC85F3`
- mit passender `HyperHideDrv.pdb`, Buildlog, Source identity, Rollback,
  Quelldiff und `SHA256SUMS.txt`.
- ausgelieferte `HyperHideDrv.pdb` SHA-256
  `C2EC9977E538CAC09AD551A12A21F574F20DFEC820E2DF5600B3BB051D2B0E41`.

CPUID-Diagnosepaket:

- `bin/HyperHide/HyperHide-CPUID-Diagnostics-x86-x64-20261029.zip`
- SHA-256 `D2BBB67F3FE312CB3B3EC4AEC1F2D05EEC4E2548302100CB290F837423F9C16A`
- enthält statisch gelinkte x86-/x64-Programme, beide PDBs, Quelle,
  reproduzierbares Buildskript, Log, Collector, Anleitung und Manifest.
- PE-Maschinentypen offline geprüft; Programme auf dem Buildrechner bewusst
  nicht ausgeführt.

Die bekannten Originale bleiben unverändert:

- v5 Source SHA-256
  `5ECD3971AA889F71D8605AACE70BF4287A40DC2ED4DA1C8997C4F7C9EB066ACD`
- v5 Runtime SHA-256
  `96831611B9D87B9222D0C2644FFE9155C162AD62D485667694EF8877602B3F7F`

## CPUID-Captures (nachgezogen, Build-PC)

Die drei Zustände von der HP-Bench sind ausgewertet:
[CPUID-CAPTURE-ANALYSIS-20261029.md](CPUID-CAPTURE-ANALYSIS-20261029.md).

- v5-airhv: `hypervisor_present=1`, Vendor `airh`, leaf `0x40000000` leakt.
- v6-airhv: Register identisch zu drivers-off (außer leaf1-EBX/APIC).
- Timing unter airhv: leaf0-Median ~756–938 Zyklen vs. ~144 nativ.

Daraus folgt: kein weiterer `0x40000000`-/Bit-31-Spoof. Ein airhv-Fix
braucht entweder Timing-Kompensation oder den per MessageBox/API-BP
identifizierten Flex-Sensor — isoliert, nicht im v5.1-Paket.

## Nächster HP-Bench-Schritt

CPUID-Collect muss nicht wiederholt werden. Als Nächstes v5.1 Runtime:

1. Paket `HyperHide-Win10-19045-x64-v5.1-ntqueryobject-fix.zip` entpacken.
2. Backup der laufenden Treiber, `preflight.ps1`, `install.ps1` (demand-start).
3. Zweistufig starten; Early-Attach **ohne** `NtQueryObject=0`.
4. Erfolg für diesen Schritt = kein `HookedNtQueryObject`-`0x3B`.
5. VM-Dialog unter airhv bleibt bis zum Sensor-/Timing-Fix erwartet.
6. Bei Crash: normal booten, Dump + mitgelieferte PDB sichern, Rollback.
