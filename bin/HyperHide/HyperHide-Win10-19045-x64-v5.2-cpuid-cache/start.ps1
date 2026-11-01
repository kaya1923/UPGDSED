#Requires -RunAsAdministrator
[CmdletBinding()]
param(
    [switch]$AirhvOnly
)

$expectedAirHash = 'C4F55AD0351B42CB6660538197AD3A7D9E7075676EC0920AC7A5900227E3DD6F'
$expectedHideHash = '2798C37855CA06B6FCA64B32E8794ED0E4D02F16793D0BA94369DB19EEA6E681'
$installedAirhv = Join-Path $env:SystemRoot 'System32\drivers\airhv.sys'
$installedHyperHide = Join-Path $env:SystemRoot 'System32\drivers\HyperHideDrv.sys'

if (-not (Test-Path -LiteralPath $installedAirhv -PathType Leaf) -or
    (Get-FileHash -LiteralPath $installedAirhv -Algorithm SHA256).Hash -ne $expectedAirHash) {
    throw 'Die korrigierte airhv.sys v2 ist nicht installiert. Zuerst install.ps1 aus diesem Bundle ausfuehren.'
}
if (-not (Test-Path -LiteralPath $installedHyperHide -PathType Leaf) -or
    (Get-FileHash -LiteralPath $installedHyperHide -Algorithm SHA256).Hash -ne $expectedHideHash) {
    throw 'Die korrigierte HyperHideDrv.sys v5.2 ist nicht installiert. Zuerst install.ps1 aus diesem Bundle ausfuehren.'
}

& (Join-Path $PSScriptRoot 'preflight.ps1')
if ($LASTEXITCODE -ne 0) {
    throw 'Vorpruefung fehlgeschlagen. Es wurde nichts gestartet.'
}

& sc.exe query airhv *> $null
if ($LASTEXITCODE -ne 0) {
    throw 'Dienst airhv fehlt. Zuerst install.ps1 ausfuehren.'
}
& sc.exe query HyperHideDrv *> $null
if ($LASTEXITCODE -ne 0) {
    throw 'Dienst HyperHideDrv fehlt. Zuerst install.ps1 ausfuehren.'
}

Write-Host 'Starte airhv ...'
& sc.exe start airhv
if ($LASTEXITCODE -ne 0) {
    throw "airhv konnte nicht gestartet werden (sc.exe: $LASTEXITCODE). HyperHideDrv wird nicht gestartet."
}

if ($AirhvOnly) {
    Write-Host 'airhv laeuft. HyperHideDrv wurde absichtlich noch nicht gestartet.' -ForegroundColor Green
    Write-Host 'Zum Beenden: powershell -ExecutionPolicy Bypass -File .\stop.ps1'
    exit 0
}

Write-Host 'Starte HyperHideDrv ...'
& sc.exe start HyperHideDrv
if ($LASTEXITCODE -ne 0) {
    $hideError = $LASTEXITCODE
    $stage = switch ($hideError) {
        1    { 'NT-Syscall-Hooks'; break }
        21   { 'VMCALL-Test: airhv nicht bereit'; break }
        126  { 'Win32k-Syscall-Hooks'; break }
        127  { 'KiDispatchException-Hook'; break }
        1167 { 'VMCALL-Test: airhv nicht verbunden'; break }
        1168 { 'SSDT-Signatur'; break }
        1306 { 'Windows-Build/Offsets'; break }
        1450 { 'Hider-Initialisierung/Speicher'; break }
        default { 'Notify-Routine oder unbekannte DriverEntry-Stufe' }
    }
    Write-Warning "HyperHideDrv konnte nicht gestartet werden (sc.exe: $hideError). airhv wird wieder gestoppt."
    Write-Warning "Diagnosestufe: $stage"
    & sc.exe stop airhv
    throw 'HyperHide-Start fehlgeschlagen.'
}

Write-Host 'airhv und HyperHideDrv laufen.' -ForegroundColor Green
