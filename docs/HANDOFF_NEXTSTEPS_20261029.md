# Handoff: v5.1 NtQueryObject fix and CPUID diagnostics

Stand: 29.10.2026

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

## Nächster HP-Bench-Schritt

Zuerst das Diagnose-ZIP entpacken und dieselben Binärdateien in drei Zuständen
verwenden:

```powershell
powershell -ExecutionPolicy Bypass -File .\collect.ps1 -State drivers-off
powershell -ExecutionPolicy Bypass -File .\collect.ps1 -State v5-airhv-only
powershell -ExecutionPolicy Bypass -File .\collect.ps1 -State v6-airhv-only
```

Vor dem zweiten und dritten Befehl jeweils ausschließlich die passende airhv
mit `start.ps1 -AirhvOnly` aus dem jeweiligen Paket starten und danach mit
dessen `stop.ps1` stoppen. Der Collector ändert selbst keinen Treiberzustand.

Für den v5.1-Runtimetest zuerst Backup/Wiederherstellung sicherstellen, v5.1
mit `install.ps1` als demand-start installieren und die vorhandene zweistufige
Startreihenfolge verwenden. Bei einem neuen Bluescreen normal booten, nicht
erneut starten und den neuen Dump sichern. Erst reale Drei-Zustands-CPUID-Daten
rechtfertigen einen isolierten airhv-Fix.
