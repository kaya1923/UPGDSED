#Requires -RunAsAdministrator
[CmdletBinding()]
param()

Write-Host 'Stoppe HyperHideDrv ...'
& sc.exe stop HyperHideDrv

Write-Host 'Stoppe airhv ...'
& sc.exe stop airhv

Write-Host 'Stop-Anforderungen wurden gesendet.' -ForegroundColor Green
