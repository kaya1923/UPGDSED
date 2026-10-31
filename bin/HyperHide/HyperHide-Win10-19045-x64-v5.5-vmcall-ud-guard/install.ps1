#Requires -RunAsAdministrator
[CmdletBinding()]
param()

$ErrorActionPreference = 'Stop'
$driverDirectory = Join-Path $env:SystemRoot 'System32\drivers'
$airSource = Join-Path $PSScriptRoot 'airhv.sys'
$hideSource = Join-Path $PSScriptRoot 'HyperHideDrv.sys'
$airTarget = Join-Path $driverDirectory 'airhv.sys'
$hideTarget = Join-Path $driverDirectory 'HyperHideDrv.sys'
$expectedAirHash = 'C4F55AD0351B42CB6660538197AD3A7D9E7075676EC0920AC7A5900227E3DD6F'
$expectedHideHash = 'DA9FA1812099310EFD30D3F05E353030EFF33B62F0CA100852E222FC7CAC9349'

foreach ($file in @($airSource, $hideSource)) {
    if (-not (Test-Path -LiteralPath $file -PathType Leaf)) {
        throw "Datei fehlt: $file"
    }
}

if ((Get-FileHash -LiteralPath $airSource -Algorithm SHA256).Hash -ne $expectedAirHash) {
    throw 'airhv.sys hat nicht den erwarteten v2-Hash. Installation abgebrochen.'
}
if ((Get-FileHash -LiteralPath $hideSource -Algorithm SHA256).Hash -ne $expectedHideHash) {
    throw 'HyperHideDrv.sys hat nicht den erwarteten v5.5-Hash. Installation abgebrochen.'
}

$existingTargets = @($airTarget, $hideTarget) | Where-Object { Test-Path -LiteralPath $_ }
if ($existingTargets.Count -gt 0) {
    $stamp = Get-Date -Format 'yyyyMMdd-HHmmss'
    $backupDirectory = Join-Path $PSScriptRoot "backup-$stamp"
    New-Item -ItemType Directory -Path $backupDirectory | Out-Null
    foreach ($target in $existingTargets) {
        Copy-Item -LiteralPath $target -Destination $backupDirectory
    }
    Write-Host "Vorhandene Treiber gesichert: $backupDirectory"
}

Copy-Item -LiteralPath $airSource -Destination $airTarget -Force
Copy-Item -LiteralPath $hideSource -Destination $hideTarget -Force

if ((Get-FileHash -LiteralPath $airTarget -Algorithm SHA256).Hash -ne $expectedAirHash) {
    throw 'Hashpruefung der installierten airhv.sys ist fehlgeschlagen.'
}
if ((Get-FileHash -LiteralPath $hideTarget -Algorithm SHA256).Hash -ne $expectedHideHash) {
    throw 'Hashpruefung der installierten HyperHideDrv.sys ist fehlgeschlagen.'
}

function Set-DemandKernelService([string]$Name, [string]$Path) {
    & sc.exe query $Name *> $null
    if ($LASTEXITCODE -eq 0) {
        & sc.exe config $Name type= kernel start= demand binPath= $Path
    }
    else {
        & sc.exe create $Name type= kernel start= demand binPath= $Path
    }
    if ($LASTEXITCODE -ne 0) {
        throw "Dienst $Name konnte nicht eingerichtet werden (sc.exe: $LASTEXITCODE)."
    }
}

Set-DemandKernelService -Name 'airhv' -Path $airTarget
Set-DemandKernelService -Name 'HyperHideDrv' -Path $hideTarget

Write-Host "Installiert (Starttyp: demand). Es wurde noch kein Treiber gestartet." -ForegroundColor Green

