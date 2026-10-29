# Nächste Schritte: HyperHide-Crash und airhv-VM-Erkennung

Stand: 29.10.2026, HP-Bench. Diese Übergabe ergänzt und aktualisiert
[docs/HANDOFF_HYPERHIDE_V5_V6.md](docs/HANDOFF_HYPERHIDE_V5_V6.md).

**Auftrag an die andere IDE:** Dieses Dokument vollständig lesen und zuerst
Prompt 1 bearbeiten. Prompt 2 ist eine getrennte Untersuchung. Vor Änderungen
vorhandene Arbeitsstände und gegebenenfalls AGENTS.md beachten.

> Bearbeitungsstand 29.10.2026 (Build-PC Nachzug): Prompt 1 und Diagnosepaket
> sind umgesetzt. Die HP-Drei-Zustands-Captures sind ausgewertet in
> [docs/CPUID-CAPTURE-ANALYSIS-20261029.md](docs/CPUID-CAPTURE-ANALYSIS-20261029.md):
> v6 matcht die CPUID-Register; verbleibend ist vor allem Timing (~3–6×).
> Kein weiterer Register-Spoof. Nächster Runtime-Schritt: v5.1 Early-Attach auf
> der HP-Bench. Runbook und ZIPs liegen im bench-min Pack unter
> `tools/flex-x32dbg/local-session/packages-20261029/`.

## Umgesetzter Stand für die nächste Maschine

Crashfix-Paket:

- `bin/HyperHide/HyperHide-Win10-19045-x64-v5.1-ntqueryobject-fix.zip`
- SHA-256 `3580804C36F27504585A670236350EC37B271E61B8A70B34D2FA4205736C95E4`
- `HyperHideDrv.sys` SHA-256
  `90974C370093984DCCEA35B6A9EBF3C54F961B35FF1BCEE28E657D4AABAC85F3`
- passende `HyperHideDrv.pdb` SHA-256
  `C2EC9977E538CAC09AD551A12A21F574F20DFEC820E2DF5600B3BB051D2B0E41`
- unveränderte v5-`airhv.sys` SHA-256
  `C4F55AD0351B42CB6660538197AD3A7D9E7075676EC0920AC7A5900227E3DD6F`

CPUID-Diagnosepaket:

- `bin/HyperHide/HyperHide-CPUID-Diagnostics-x86-x64-20261029.zip`
- SHA-256 `D2BBB67F3FE312CB3B3EC4AEC1F2D05EEC4E2548302100CB290F837423F9C16A`
- enthält x86/x64 EXE und PDB, Quelle, Buildskript, Buildlog,
  `collect.ps1`, Anleitung und `SHA256SUMS.txt`.

Auf der HP-Bench mit denselben Diagnosebinärdateien erfassen:

```powershell
powershell -ExecutionPolicy Bypass -File .\collect.ps1 -State drivers-off
powershell -ExecutionPolicy Bypass -File .\collect.ps1 -State v5-airhv-only
powershell -ExecutionPolicy Bypass -File .\collect.ps1 -State v6-airhv-only
```

Vor dem zweiten beziehungsweise dritten Lauf nur die passende `airhv.sys` über
`start.ps1 -AirhvOnly` starten und anschließend mit deren `stop.ps1` stoppen.
Der Collector ändert selbst keinen Treiberzustand. Der v5.1-Treiber wurde
offline gebaut und geprüft, aber noch nicht auf der HP-Bench geladen. Beide
ZIPs wurden nach dem Packen zurückentpackt und vollständig gegen ihre
SHA-256-Manifeste geprüft. Vollständige technische Übergabe:
[docs/HANDOFF_NEXTSTEPS_20261029.md](docs/HANDOFF_NEXTSTEPS_20261029.md).

## Verifizierte Ausgangslage

- HP EliteBook 840 G1, Intel i5-4300U, Windows 10 22H2 Build 19045,
  Kernel 10.0.19041.7802, UPGDSED-Kernelalias `ntkrnlmp.exe`.
- VT-x/EPT aktiv, Microsoft-Hypervisor und VBS aus. Flex 7.59.1.0.
- Beim frühen Attach am **28.10.2026 um 21:38:27.882 CET**: Bluescreen
  `0x3B / 0xC0000005`, `nt!RtlEqualUnicodeString+0x43`, Lesen von `0x80`.
  Aufrufer `HyperHideDrv+0x3B26`, Funktion beginnt bei `+0x3A30`:
  `HookedNtQueryObject`, Klasse 2 (`ObjectTypeInformation`).
- `ObjectInformation=0x0528FA70`, `ReturnLength=0x0528FA78`:
  Die Rückgabelänge überlappt das native `UNICODE_STRING.Buffer`-Feld.
  Der Mittagsdump enthält denselben Fehler im selben Treiberbuild.
- Mit `NtQueryObject=0` kam ein erneuter früher Attach durch,
  `BeingDebugged=0`; danach erschien bei 75 % der VM-Dialog.
- **Auch ohne Debugger, nur mit airhv und gestopptem HyperHideDrv,
  erscheint der VM-Dialog.** Ohne beide Treiber erreicht Flex wieder die
  Hauptoberfläche. Der konkrete VM-Sensor ist noch nicht identifiziert.
- Am Ende der Bench-Prüfung waren beide Treiber gestoppt, Flex in der
  Hauptoberfläche und kein Flex-Systemdialog mehr offen.

Der frühere stabile Halt bei `+995900` war ein später Attach und beweist
keinen erfolgreichen Start mit Debugger. Die Bezeichnung „known-good v5“
im alten Handoff belegt nur die Treiberinitialisierung.

Geprüfter Quellstand vor dieser Übergabe:
`57643d5b3d1f085b35b7a92e9fcb894490539cfd`.
Der aktuelle Vendor-Baum enthält bereits v6-Änderungen. Für einen isolierten
Crash-Fix den gesonderten v5-Quellsnapshot verwenden, nicht unbemerkt v6
mitbauen. Auf der Bench fehlen MSVC und die C++-/WDK-Build-Komponenten;
es wurden hier keine neuen Treiber gebaut.

## Belege und Offline-Nachprüfung

Alle übergebenen Belege liegen in
[docs/bench-20261029/](docs/bench-20261029/):

- [Ausführliche Crash-Analyse](docs/bench-20261029/CRASH-20261028-213827.md).
- [Neuster Minidump](docs/bench-20261029/102826-7203-01.dmp), Crash 21:38:27 CET.
- [Mittags-Minidump](docs/bench-20261029/102826-6828-01.dmp), Crash 12:20:12 CET.
- `minidump-analysis.txt`, `midday-dump-analysis.txt`:
  CDB-Ausgabe von `!analyze -v`, Kontext, Stack und geladenen Modulen.
- `full-dump-analysis.txt`, `queryobject-detail.txt`:
  zusätzlich aus dem lokalen Kernel-Bitmap-Dump gewonnene Disassemblierung,
  Unwind-Informationen und vollständigerer Kernelstack. Der große
  `MEMORY.DMP` verbleibt auf der Bench; beide kleinen Minidumps sind enthalten.
- `minidump-with-v5-image.txt`: erneute Offline-Nachprüfung nur mit dem
  übergebenen Minidump und dem Treiberimage aus dem archivierten v5-ZIP.
  Fehlerstelle, Funktion bei +0x3A30 und Hook-Disassemblierung reproduziert.
- `native-airhv-only-evidence.json`: airhv Running, HyperHideDrv Stopped,
  betroffener Prozess und neuer VM-Dialog unter csrss.exe.
- `native-with-drivers-dialog.json`: Dialog des nativen Starts mit beiden Treibern.
- `final-state-without-drivers.json`: beide Dienste gestoppt, Hauptoberfläche
  `MagicMotorSport Flex`, keine Flex-Systemdialoge.
- `HyperHide-mitigated.ini`: Bench-Profil mit `NtQueryObject=0`.
- `SHA256SUMS.txt`: Hashes der Belegdateien.

Die beiden Minidumps wurden auf ausdrücklichen Wunsch des Eigentümers für
diese private Übergabe aufgenommen. Die Git-ignore-Ausnahmen gelten nur
für diese zwei Dateien; weitere Dumps und Symbolcaches bleiben ausgeschlossen.

Nach Möglichkeit Minidump und v5-Treiberimage gemeinsam verwenden:

```powershell
# Aus dem Repository-Root. Debuggerpfad bei Bedarf an die Build-IDE anpassen.
$cdb = 'C:\Program Files (x86)\Windows Kits\10\Debuggers\x64\cdb.exe'
$v5 = Join-Path $env:TEMP ('HyperHide-v5-crash-repro-' + [guid]::NewGuid().ToString('N'))
Expand-Archive -LiteralPath '.\bin\HyperHide\known-good\HyperHide-Win10-19045-x64-v5-KNOWN-GOOD.zip' -DestinationPath $v5
$image = Get-ChildItem -LiteralPath $v5 -Recurse -Filter HyperHideDrv.sys | Select-Object -First 1
if (-not $image) { throw 'v5 driver image not found' }
if ((Get-FileHash -LiteralPath $image.FullName).Hash -ne 'C9DC14B85020B32E4D08DC22B66B6FC7F5E247FDC73CD062B0D7352B57AB8D57') { throw 'Wrong driver build' }
$symbols = 'srv*' + (Join-Path $PWD 'symbols') + '*https://msdl.microsoft.com/download/symbols'
& $cdb -z '.\docs\bench-20261029\102826-7203-01.dmp' -i $image.DirectoryName -y $symbols -c '!analyze -v; .cxr 0xffffb9002021afc0; kv; .fnent HyperHideDrv+0x3b26; uf HyperHideDrv+0x3b26; lmvm HyperHideDrv; q'
```

Für den Mittagsdump ist der Kontext `0xffff820784a6efc0`.
Minidumps enthalten nicht alle Benutzerseiten. Daraus fehlende Pufferinhalte
nicht als sicher bekannt darstellen. Die mitgelieferten Treiberimages
aus dem v5-ZIP helfen bei Code und Unwind-Daten; passende private PDBs des
neuen Builds sind für die nächste Übergabe erforderlich.

Native Starttests und Treiber-Ladetests finden auf der HP-Bench statt,
nicht auf dem Entwicklungsrechner. Alte csrss-Fehlerdialoge können nach
Prozessende weiter bestehen; Handles und Prozesszuordnung mitprüfen.

## Prompt 1: HyperHideDrv-Crash auf v5-Basis beheben

Den folgenden Auftrag zuerst vollständig bearbeiten:

```text
Arbeite im Repository https://github.com/kaya1923/UPGDSED-HyperHide-private.
Ziel: einen gezielten HyperHideDrv-Fix auf dem Build-PC vorbereiten und bauen.
Keine Treiber auf dem Build-PC laden; keine BCD-, Kernel-, DSE- oder
Signaturoptionen aendern. Runtime-Tests erfolgen spaeter auf der HP-Bench.

Zuerst docs/HANDOFF_HYPERHIDE_V5_V6.md und vorhandene AGENTS.md lesen.
Von der Bench gepruefter Repository-Stand:
57643d5b3d1f085b35b7a92e9fcb894490539cfd, Branch master.
Vor Arbeiten aktuelle lokale Aenderungen und Branch pruefen, nichts verwerfen.

WICHTIG ZUR BASIS:
vendor/HyperHide enthaelt in diesem Stand bereits v6-Aenderungen.
Fuer den isolierten Crash-Fix den v5-Quellstand aus
bin/HyperHide/known-good/HyperHide-v5-source-KNOWN-GOOD.zip verwenden.
ZIP-SHA256: 5ECD3971AA889F71D8605AACE70BF4287A40DC2ED4DA1C8997C4F7C9EB066ACD.
V5-/known-good-Artefakte unveraendert lassen; in eigener Arbeitskopie arbeiten.
Die Bezeichnung known-good bestaetigt nur die Treiberinitialisierung,
nicht fehlerfreien fruehen Flex-Start oder stabile Debug-Hooks.

NEUE DUMP-EVIDENZ VON DER BENCH:
HP EliteBook 840 G1, Intel i5-4300U, Win10 22H2 Build 19045,
Kernel 10.0.19041.7802, UPGDSED-Kernelalias ntkrnlmp.exe.
Minidump 102826-7203-01.dmp; Crash 28.10.2026 21:38:27.882 CET.
0x3B SYSTEM_SERVICE_EXCEPTION, Exception 0xC0000005, Prozess flex.exe.
nt!RtlEqualUnicodeString+0x43 liest von 0x80.
Aufrufer HyperHideDrv+0x3B26, Funktionsanfang +0x3A30.
Die vollstaendige Disassemblierung identifiziert HookedNtQueryObject,
ObjectTypeInformation (Klasse 2), Vergleich mit L"DebugObject".
Originalstatus 0. Unwind-Flags der Hook-Funktion: 0, kein eigener SEH-Handler.
Der Mittagsdump 102826-6828-01.dmp zeigt denselben Fehler und denselben Build.

Register aus dem Hook-Prolog/Fehlerkontext:
RDI = ObjectInformation = 0x0528FA70
RSI = ReturnLength      = 0x0528FA78
R14D = ObjectInformationClass = 2
R15D = Originalstatus = 0
RAX = gelesener TypeName.Buffer = 0x80
ReturnLength ueberlappt somit ObjectInformation+8, das Buffer-Feld der
nativen UNICODE_STRING. Das passt zur Ueberschreibung des Zeigers mit
einer Rueckgabelaenge 0x80. Der genaue Schreibzeitpunkt ist nicht im Dump
belegt, weil Benutzerseiten teilweise fehlen; diese Grenze beibehalten.

Absturztreiber:
HyperHideDrv.sys SHA256
C9DC14B85020B32E4D08DC22B66B6FC7F5E247FDC73CD062B0D7352B57AB8D57
airhv.sys SHA256
C4F55AD0351B42CB6660538197AD3A7D9E7075676EC0920AC7A5900227E3DD6F

AUFGABE:
1. vendor/HyperHide/HyperHideDrv/HookedFunctions.cpp,
   HookedNtQueryObject pruefen (im untersuchten master ab Zeile 551).
   Die ungesicherten RtlEqualUnicodeString-Aufrufe fuer Klasse 2 und 3
   sind im passenden v5-Snapshot zu bestaetigen.
2. Minimalen Fix fuer die gesamte Nachbearbeitung von Benutzerpuffern
   erstellen: sichere Laengen-/Bereichs-/Ueberlaufpruefung, abgesicherte
   Erfassung von UNICODE_STRING und Inhalt, SEH fuer ALLE Benutzerzugriffe
   einschliesslich ReturnLength, Zaehler und Pointer-Walk in Klasse 3.
   ProbeForRead/Write allein beseitigt TOCTOU nicht. Keine ungesicherten
   Benutzerzeiger an RtlEqualUnicodeString weiterreichen.
3. Ueberlappung von ReturnLength und ObjectInformation explizit behandeln.
   Wenn nach dem Originalaufruf keine sichere Nachbearbeitung moeglich ist,
   dessen Rueckgabe erhalten und die Nachbearbeitung auslassen. Keine
   erfolgreiche Originalantwort allein wegen des nachgeschalteten Hooks
   in einen neuen Fehler umwandeln. Originalaufruf nicht durch Verwendung
   anderer Puffer unbemerkt semantisch veraendern.
4. Native Kernel-Strukturlayouts und WOW64-Konvertierung getrennt beachten.
   Nicht pauschal aufgrund eines 32-Bit-Aufrufers UNICODE_STRING32 im
   x64-Kernelhook annehmen. Keine fest geratenen Offsets als Reparatur.
5. Die bestaetigten v5-Fixes fuer VPID-Invalidierung, Kernelalias,
   KiDispatchException-Signatur und Build-Offsets unveraendert erhalten.
   airhv und CPUID-/Plugin-Lifecycle fuer diesen Fix NICHT veraendern.

VALIDIERUNG UND BUILD:
- Regressionen fuer Klasse 2/3: normale Puffer, ReturnLength=NULL,
  getrennte Rueckgabe, Rueckgabe im Output (insbesondere +8), kurze Puffer,
  ungueltige/veraenderte Stringzeiger, Grenzueberlaeufe und konkurrierende
  Aenderungen. Bestehenden fehlerhaften Treiber nicht fuer solche Live-Tests
  laden. Offlinepruefungen und noch ausstehende HP-Runtime-Tests ausweisen.
- Bekannte Umgebung laut Handoff: VS Community 18, MSVC 14.51.36231,
  SDK/WDK 10.0.26100.0, auch SDK 10.0.19041.0. Bestehende explizite
  Compiler-/Include-/Lib-/Linker-Pfade des Projekts ermitteln und benutzen.
  Root-build.ps1 baut src/patch.sln / patch.exe, NICHT HyperHideDrv!
- Release-x64-Treiber mit passenden PDBs erzeugen. Reproduzierbaren
  Build-Befehl oder Build-Skript und vollstaendiges Build-Log ablegen.
- Separates Paket v5.1-ntqueryobject-fix liefern: neue HyperHideDrv.sys,
  passende PDB, unveraenderte v5-airhv.sys mit Hashpruefung, SHA256SUMS,
  Quell-Diff, Basis-Commit/Snapshot-Hash, Install-/Start-/Stop-Skripte,
  Backup-/Rueckfallpfad. Keine automatische Installation oder Neustarts.
- Die zwei autorisierten Minidumps unter docs/bench-20261029 fuer die Offlineanalyse verwenden; weitere Speicherdumps nicht automatisch hinzufuegen. Den alten
  v5-Build und seine ZIPs nicht ueberschreiben.

WICHTIGE ABGRENZUNG:
NtQueryObject=0 in den Profilen Default Attach / Default Create Process
verhindert auf der Bench den beobachteten Hook-Crash. Der fruehe Attach
kam damit durch, BeingDebugged=0; danach kam bei 75% ein VM-Dialog.
Dieser Dialog tritt sogar beim nativen Start mit NUR airhv auf.
Der Crash-Fix alleine loest deshalb noch nicht den vollstaendigen Start.
Die VM-Erkennung wird getrennt in PROMPT-02 untersucht.

Am Ende melden: geaenderte Stellen, Build-Ergebnis, ausgefuehrte Tests,
noch NICHT ausgefuehrte Tests und Pfad/Hash des zur Bench zu kopierenden
Pakets. Nicht behaupten, Flex starte erfolgreich, solange das nicht auf
der HP-Bench geprueft wurde.
```

## Prompt 2: airhv-VM-Erkennung getrennt untersuchen

Die Crash-Reparatur nicht mit ungemessenen CPUID-Änderungen vermischen:

```text
Arbeite im Repository https://github.com/kaya1923/UPGDSED-HyperHide-private.
Dies ist die zweite, vom NtQueryObject-Crash getrennte Baustelle.
Lies docs/HANDOFF_HYPERHIDE_V5_V6.md und vorhandene AGENTS.md.
Gepruefter Stand: 57643d5b3d1f085b35b7a92e9fcb894490539cfd.
Auf dem Build-PC nur Code bearbeiten und bauen; keine Treiber laden,
kein UPGDSED ausfuehren, keine BCD-/Kernel-/DSE-Aenderungen.

NEUE RUNTIME-BEFUNDE VON DER HP-BENCH:
HP EliteBook 840 G1, Intel i5-4300U, Win10 22H2 Build 19045,
Kernel 10.0.19041.7802. Hyper-V/VBS aus, VT-x/EPT an.
Flex FileVersion/ProductVersion 7.59.1.0.

A: Beide Treiber gestoppt, nativer Start ohne Debugger:
   Flex erreicht Hauptoberflaeche "MagicMotorSport Flex".
B: v5-airhv UND v5-HyperHideDrv geladen, nativer Start ohne Attach:
   "Sorry, this application cannot run under a Virtual Machine."
C: NUR v5-airhv geladen, HyperHideDrv gestoppt, kein Debugger:
   Neuer gleicher VM-Dialog; nach Bestaetigung endet der betreffende
   flex.exe-Prozess. Die VM-Erkennung haengt somit nicht am Attach-Hook.
D: Frueher Attach mit beiden Treibern, NtQueryObject=0:
   BeingDebugged=0, kein erneuter Bluescreen beobachtet, VM-Dialog bei 75%.

Die beteiligte airhv.sys ist der lokale v5-Build mit SHA256
C4F55AD0351B42CB6660538197AD3A7D9E7075676EC0920AC7A5900227E3DD6F.
V6 wurde frueher ebenfalls mit VM-Dialog gemeldet; damit ist NICHT
nachgewiesen, dass ausschliesslich v6 die Erkennung verursacht.
Noch unbekannt: die konkrete von Flex ausgewertete VM-Pruefung.

QUELLSTAND:
- v5-Snapshot: bin/HyperHide/known-good/HyperHide-v5-source-KNOWN-GOOD.zip
  SHA256 5ECD3971AA889F71D8605AACE70BF4287A40DC2ED4DA1C8997C4F7C9EB066ACD.
- Aktuelles vendor/HyperHide enthaelt bereits v6-Varianten.
- airhv/airhv/vmm.cpp setzt im aktuellen Stand hv_presence=false.
- airhv/airhv/vmexit_handler.cpp, vmexit_cpuid_handler ab etwa Zeile 522:
  Im Hidden-Fall liefern 0x40000000..0x4FFFFFFF die Ergebnisse von
  highest_basic_leaf, Subleaf 0. Ob das zu nativer Invalid-Leaf-Semantik
  dieser CPU passt, ist zu MESSEN, nicht zu behaupten.
- HyperHide/pluginmain.cpp schaltet die Sichtbarkeit im Menue und wurde
  fuer v6 beim Plugin-Lifecycle geaendert. Auch diese Zustandswechsel
  getrennt vom airhv-only-Fall pruefen.

AUFGABE:
1. Zuerst ein reproduzierbares Diagnosepaket bauen. Vorhandenes
   vendor/HyperHide/tools/cpuid_verify.cpp pruefen und bei Bedarf ergaenzen.
   Native CPUID-Werte (Eingangsleaf UND Subleaf, alle vier Register) fuer
   relevante Basic-/Extended-/Hypervisor-Grenzwerte protokollieren,
   insbesondere Leaf 0, 1, 7/0, hoechste Basic-/Extended-Leaf,
   0x40000000 und 0x40000001 sowie ungueltige Leaves an den Grenzen.
   Vorhandene Timingmessungen mit Stichprobenzahl und Streuung ausgeben;
   aus einzelnen Timingwerten keine Diagnose ableiten.
2. x86- und x64-Diagnoseprogramme sowie dokumentierten Ablauf fuer die
   HP-Bench liefern. Gleiche Programme/Versionen in den Zustaenden
   Treiber aus, v5-airhv-only, danach gegebenenfalls v6-airhv-only nutzen.
   Reale Messdaten von der Bench abwarten, wenn sie fuer eine Aenderung
   erforderlich sind. Keine unbekannten CPU-Antworten erfinden.
3. VM-Exit-Handler und Sichtbarkeitszustand anhand dieser Daten pruefen.
   Minimalen, begruendeten Fix isolieren; eine Variable pro Variante.
   Nicht erneut blind CPUID-Bits, Vendorstrings, Systemuhren, Windows-
   Offsets oder Plugin-Injektion gleichzeitig veraendern.
4. Erforderliche VPID-/Kernelalias-/KiDispatchException-Fixes erhalten.
   Den unabhaengigen NtQueryObject-Crashfix nicht durch Rebuild des
   gesamten alten Bundles versehentlich zuruecknehmen.

BENCH-TESTREGELN FUER DAS PAKET:
- Erst nativer Flex-Start ohne Debugger mit nur airhv. Er muss bis zur
  Hauptoberflaeche ohne VM-Dialog kommen, bevor ein Attach-Test sinnvoll ist.
- Dienste demand-start belassen. Kein automatischer Neustart/BCD-Eingriff.
- System-Hard-Error-Dialoge erscheinen unter csrss.exe, NICHT unbedingt
  unter flex.exe. Alte Dialoge koennen nach Prozessende stehenbleiben und
  neue Dialoge aufstauen. Fensterhandles vor/nach Test, Dialogtext und
  betroffenen Prozess erfassen; alte Dialoge explizit unterscheiden.
  Der beobachtete OK-Button hatte Control-ID 2.
- Ein lebender Prozess, SERVICE_RUNNING oder BeingDebugged=0 alleine
  belegen keinen erfolgreichen Startup.
- Ein neuer Kernel-Crash beendet weitere Live-Versuche bis zur Dumpanalyse.

BUILD/UEBERGABE:
Die dokumentierte VS-/MSVC-/SDK-/WDK-Umgebung des Build-PCs nutzen.
airhv-Konfiguration Release_Minimal. Root-build.ps1 baut nur UPGDSED,
nicht die Treiber. Reproduzierbare Treiber-/Tool-Build-Skripte liefern.
V5-Originale und known-good-ZIPs unveraendert lassen. Neue Varianten in
separaten Paketen mit Quellstand, Diff, Build-Log, passenden PDBs,
SHA256SUMS und den tatsaechlich ausgefuehrten Tests ablegen.
Offene Messfragen und ungetestete Eigenschaften ausdruecklich nennen.
Die zwei autorisierten Minidumps unter docs/bench-20261029 fuer die Offlineanalyse verwenden; weitere Speicherdumps nicht automatisch hinzufuegen.

Ergebnis fuer mich: Diagnoseprogramme + kurze genaue Bench-Befehle;
anschliessend, wenn die Evidenz reicht, ein getrenntes korrigiertes
airhv-Paket. Kein Build auf Verdacht als "Flex-kompatibel" deklarieren.
```

## Erwartete Rückgabe an die Bench

1. Zuerst ein eigenständiges Crash-Fix-Paket mit neuer HyperHideDrv.sys,
   passenden PDBs, unveränderter v5-airhv.sys, Quell-Diff, reproduzierbarem
   Build, Build-Log und SHA256SUMS. Original-v5-Dateien erhalten.
2. Separat das Diagnosepaket für airhv, inklusive x86-/x64-Prüfprogrammen
   und genauen Befehlen zum Erfassen der noch fehlenden Bench-Messwerte.
3. Ausgeführte Offlineprüfungen und ausstehende Runtimeprüfungen unterscheiden.
   Keine erfolgreiche Flex-Kompatibilität allein aus einem erfolgreichen
   Build, SERVICE_RUNNING oder BeingDebugged=0 ableiten.
4. Neue Artefakte und aktualisierte Übergabe im Repository benennen; keine
   automatisch gestarteten Installer, Reboots oder Bootänderungen.
