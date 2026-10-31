#Requires -RunAsAdministrator
[CmdletBinding()]
param(
    [switch]$DeleteDriverFiles
)

& (Join-Path $PSScriptRoot 'stop.ps1')

foreach ($service in @('HyperHideDrv', 'airhv')) {
    & sc.exe delete $service
}

if ($DeleteDriverFiles) {
    foreach ($name in @('HyperHideDrv.sys', 'airhv.sys')) {
        $path = Join-Path (Join-Path $env:SystemRoot 'System32\drivers') $name
        if (Test-Path -LiteralPath $path) {
            Remove-Item -LiteralPath $path -Force
        }
    }
    Write-Host 'Dienste und installierte Treiberdateien entfernt.' -ForegroundColor Green
}
else {
    Write-Host 'Dienste entfernt. Treiberdateien wurden beibehalten.' -ForegroundColor Green
}
