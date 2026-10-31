#Requires -RunAsAdministrator
[CmdletBinding()]
param()

function Wait-ServiceState([string]$Name, [string]$Desired, [int]$TimeoutSec = 30) {
    $deadline = (Get-Date).AddSeconds($TimeoutSec)
    do {
        $out = & sc.exe query $Name 2>$null | Out-String
        if ($out -match ("STATE\s*:\s*\d+\s+" + [regex]::Escape($Desired))) {
            return $true
        }
        Start-Sleep -Milliseconds 200
    } while ((Get-Date) -lt $deadline)
    return $false
}

Write-Host 'Stoppe HyperHideDrv ...'
& sc.exe stop HyperHideDrv | Out-Null
if (-not (Wait-ServiceState -Name 'HyperHideDrv' -Desired 'STOPPED')) {
    Write-Warning 'HyperHideDrv nicht innerhalb 30s STOPPED — airhv wird trotzdem gestoppt.'
} else {
    Write-Host 'HyperHideDrv ist STOPPED.'
}

Write-Host 'Stoppe airhv ...'
& sc.exe stop airhv | Out-Null
if (-not (Wait-ServiceState -Name 'airhv' -Desired 'STOPPED')) {
    Write-Warning 'airhv nicht innerhalb 30s STOPPED.'
} else {
    Write-Host 'airhv ist STOPPED.'
}

Write-Host 'Stop-Sequenz abgeschlossen (HyperHide vor airhv).' -ForegroundColor Green
