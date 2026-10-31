#Requires -RunAsAdministrator
[CmdletBinding()]
param()

$currentEntry = '{current}'

Write-Host 'Aktuell gebooteter Windows-Eintrag:'
& bcdedit.exe /enum $currentEntry
if ($LASTEXITCODE -ne 0) {
    throw 'Der aktuelle BCD-Eintrag konnte nicht gelesen werden.'
}

& bcdedit.exe /default $currentEntry
if ($LASTEXITCODE -ne 0) {
    throw 'Der aktuelle BCD-Eintrag konnte nicht als Standard gesetzt werden.'
}

& bcdedit.exe /displayorder $currentEntry /addfirst
if ($LASTEXITCODE -ne 0) {
    throw 'Der aktuelle BCD-Eintrag konnte nicht an die erste Stelle verschoben werden.'
}

Write-Host 'Der aktuell gebootete Eintrag ist jetzt Standard und steht im Bootmenue an erster Stelle.' -ForegroundColor Green
