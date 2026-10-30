HyperHide x64 test bundle - Windows 10 22H2 (Build 19045)

Build v2 (Dump-Korrektur fuer HP EliteBook 840 G1 / Haswell):
- airhv als Release_Minimal gebaut
- VPID-Invalidierung liest den aktiven VPID aus dem VMCS
- Start kann mit -AirhvOnly in zwei Stufen getestet werden

Build v3 (Windows 10 19041.7802):
- veraltete KiDispatchException-Callsite-Signatur ersetzt
- neue direkte Signatur gegen Microsoft-PDB verifiziert (RVA 0x305BE0)
- HyperHideDrv-Startskript akzeptiert nur den korrigierten v3-Hash

Build v4 (DriverEntry-Diagnose):
- eindeutige Fehlercodes fuer VMCALL, Offsets, SSDT, Hider und Hook-Stufen
- start.ps1 zeigt bei einem Ladefehler die genaue Diagnosestufe an

Build v5 (UPGDSED-Kernelalias):
- der laufende Kernel wird ueber den ersten SystemModuleInformation-Eintrag ermittelt
- funktioniert auch, wenn der UPGDSED-Booteintrag ntkrnlmp.exe statt ntoskrnl.exe laedt
- behebt den diagnostizierten Startfehler 1168 in der SSDT-Stufe

Build v5.1 (isolierter NtQueryObject-Crashfix):
- basiert auf dem eingefrorenen und auf dem Zielsystem lauffaehigen v5-Quellstand
- airhv.sys, Plugin, Offsets, Kernelalias und KiDispatch-Signatur sind unveraendert
- verwirft die Nachbearbeitung, wenn ReturnLength den ObjectInformation-Puffer ueberlappt
- verarbeitet ObjectTypeInformation/ObjectTypesInformation nur aus validierten Snapshots
- alle verbleibenden Benutzerpufferzugriffe sind mit Bereichspruefung und SEH abgesichert
- enthaelt HyperHideDrv.pdb sowie Buildlog und Quelldiff unter evidence\

Build v5.2 (KUSER_SHARED_DATA-Lebenszyklus):
- basiert unveraendert auf v5.1; airhv.sys, Plugin und NtQueryObject-Fix bleiben erhalten
- speichert und restauriert den kompletten KUSER_SHARED_DATA-PTE atomar
- invalidiert nach jedem PTE-Wechsel die TLBs aller Prozessoren
- restauriert den Original-PTE vor dem Freigeben der privaten Seite
- verwendet dieselbe sichere Aufraeumroutine fuer Profilwechsel, Prozessende und Treiber-Unload
- prueft Allokation und PTE-Ermittlung und meldet Hookfehler an das Plugin zurueck
- die bestehende, noch nicht vollstaendig verstandene MmPfnDatabase-Bitbehandlung wurde NICHT veraendert

Geprueft gegen ntoskrnl.exe 10.0.19041.7802 und den passenden Microsoft-PDB:
- _KTHREAD.MiscFlags: 0x74
- _ETHREAD.CrossThreadFlags: 0x510
- _ETHREAD.PicoContext: 0x5E0
- _EPROCESS.Flags2: 0x460
- _EPROCESS.SeAuditProcessCreationInfo: 0x5C0

WICHTIG
- Nur auf dem HP-Testsystem verwenden, nicht auf der Entwicklungsmaschine.
- Intel VT-x und EPT muessen im BIOS AN sein.
- Hyper-V, Windows Hypervisor Platform und VBS muessen AUS sein.
- Im gepatchten UPGDSED-Booteintrag starten, weil die Treiber unsigniert sind.
- Alle Dienste sind DEMAND_START. Es wird kein Autostart eingerichtet.
- v5.2 ist offline gebaut und getestet, aber noch nicht auf dem Zielsystem laufzeitgetestet.
- v5.1-fixed bleibt separat als Rueckfallversion erhalten.
- Vor dem Stoppen alle x64dbg/x32dbg-Instanzen schliessen.

ABLAUF (PowerShell als Administrator)
1. powershell -ExecutionPolicy Bypass -File .\preflight.ps1
2. powershell -ExecutionPolicy Bypass -File .\install.ps1
3. Wiederherstellungspunkt/Backup pruefen und offene Arbeit speichern.
4. Erste Stufe, nur airhv:
   powershell -ExecutionPolicy Bypass -File .\start.ps1 -AirhvOnly
5. Erst nach erfolgreichem airhv-Test wieder stoppen:
   powershell -ExecutionPolicy Bypass -File .\stop.ps1
6. Fuer den vollstaendigen Start:
   powershell -ExecutionPolicy Bypass -File .\start.ps1
7. HyperHide.dp64 und HyperHide.ini nach <x64dbg>\release\x64\plugins kopieren.
8. x64dbg starten und die HyperHide-Einstellungen kontrollieren.

STOPPEN
  powershell -ExecutionPolicy Bypass -File .\stop.ps1

ENTFERNEN
  powershell -ExecutionPolicy Bypass -File .\remove.ps1

Auch Treiberdateien entfernen:
  powershell -ExecutionPolicy Bypass -File .\remove.ps1 -DeleteDriverFiles

AKTUELL GEBOOTETEN BCD-EINTRAG ZUM STANDARD/ERSTEN EINTRAG MACHEN
  powershell -ExecutionPolicy Bypass -File .\make-current-boot-default.ps1

Wenn 'sc start airhv' fehlschlaegt, HyperHideDrv nicht manuell starten.
Fehler 577 bedeutet normalerweise, dass DSE noch aktiv ist bzw. der falsche
Booteintrag verwendet wurde. Bei einem Bluescreen den normalen Booteintrag
verwenden und keine weiteren Startversuche machen, bis der Dump ausgewertet ist.
v5.2 nur nach einem vollstaendigen Neustart testen; keine wiederholten
airhv/HyperHideDrv-Start-Stopp-Zyklen im selben Boot durchfuehren.
