[CmdletBinding()]
param(
    [ValidateSet('Debug', 'Release')]
    [string]$Configuration = 'Release'
)

$ErrorActionPreference = 'Stop'
$root = $PSScriptRoot
$solution = Join-Path $root 'src\patch.sln'
$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'

if (-not (Test-Path -LiteralPath $vswhere -PathType Leaf)) {
    throw 'vswhere.exe was not found. Install Visual Studio with C++ tools.'
}

$installation = & $vswhere -latest -products * -requires Microsoft.Component.MSBuild -property installationPath
if (-not $installation) {
    throw 'No Visual Studio installation with MSBuild was found.'
}

$msbuild = Join-Path $installation 'MSBuild\Current\Bin\MSBuild.exe'
if (-not (Test-Path -LiteralPath $msbuild -PathType Leaf)) {
    throw "MSBuild was not found at $msbuild"
}

& $msbuild $solution `
    /m `
    /t:Rebuild `
    "/p:Configuration=$Configuration" `
    /p:Platform=x64 `
    /p:PlatformToolset=v145 `
    /v:minimal

if ($LASTEXITCODE -ne 0) {
    throw "Build failed with exit code $LASTEXITCODE"
}

$output = Join-Path $root "src\output\x64\$Configuration\patch.exe"
if (Test-Path -LiteralPath $output -PathType Leaf) {
    Write-Host "Built: $output" -ForegroundColor Green
}
else {
    Write-Warning "MSBuild succeeded, but the expected output was not found at $output"
}
