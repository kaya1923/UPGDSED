[CmdletBinding()]
param()

$ErrorActionPreference = 'Stop'
$failures = [System.Collections.Generic.List[string]]::new()

try {
    $os = Get-CimInstance Win32_OperatingSystem
    $cpu = Get-CimInstance Win32_Processor | Select-Object -First 1
    $system = Get-CimInstance Win32_ComputerSystem
    $deviceGuard = Get-CimInstance -Namespace root\Microsoft\Windows\DeviceGuard -ClassName Win32_DeviceGuard
}
catch {
    Write-Error "Systemstatus konnte nicht abgefragt werden: $($_.Exception.Message)"
    exit 2
}

Write-Host "Windows-Build:                 $($os.BuildNumber)"
Write-Host "CPU:                           $($cpu.Name)"
Write-Host "Virtualisierung in Firmware:   $($cpu.VirtualizationFirmwareEnabled)"
Write-Host "VMX-Erweiterungen:             $($cpu.VMMonitorModeExtensions)"
Write-Host "SLAT/EPT:                      $($cpu.SecondLevelAddressTranslationExtensions)"
Write-Host "Microsoft-Hypervisor aktiv:    $($system.HypervisorPresent)"
Write-Host "VBS-Status:                    $($deviceGuard.VirtualizationBasedSecurityStatus)"

if ($os.BuildNumber -ne '19045') {
    $failures.Add("Dieses Bundle wurde nur fuer Windows 10 Build 19045 geprueft.")
}
if ($cpu.Manufacturer -notmatch 'GenuineIntel') {
    $failures.Add("HyperHide/airhv unterstuetzt nur Intel-Prozessoren.")
}
if (-not $cpu.VirtualizationFirmwareEnabled) {
    $failures.Add("Intel VT-x ist im BIOS nicht aktiv.")
}
if (-not $cpu.VMMonitorModeExtensions) {
    $failures.Add("Die VMX-Erweiterungen sind nicht verfuegbar.")
}
if (-not $cpu.SecondLevelAddressTranslationExtensions) {
    $failures.Add("SLAT/EPT ist nicht verfuegbar.")
}
if ($system.HypervisorPresent) {
    $failures.Add("Der Microsoft-Hypervisor laeuft noch. Hyper-V/VBS wirklich abschalten und neu starten.")
}
if ($deviceGuard.VirtualizationBasedSecurityStatus -ne 0) {
    $failures.Add("VBS ist noch aktiv (erwartet: Status 0).")
}

if ($failures.Count -gt 0) {
    Write-Host ""
    Write-Host "NICHT STARTEN:" -ForegroundColor Red
    $failures | ForEach-Object { Write-Host " - $_" -ForegroundColor Red }
    exit 1
}

Write-Host ""
Write-Host "Vorpruefung bestanden. airhv kann testweise gestartet werden." -ForegroundColor Green
exit 0
