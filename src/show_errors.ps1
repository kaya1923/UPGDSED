$log = Get-Content 'G:\PROJECTS\UPGDSED\src\build_errors.log' -ErrorAction SilentlyContinue
if (-not $log) { Write-Output "NO LOG FILE"; exit }
Write-Output ('Total lines: ' + $log.Count)
Write-Output "---- ERRORS ----"
$log | Where-Object { $_ -match 'error C|error LNK|fatal' } | Select-Object -First 120
Write-Output "---- LAST 40 LINES ----"
$log | Select-Object -Last 40
