[CmdletBinding()]
param(
    [string]$OutputDirectory = '',
    [string]$WindowsSdkVersion = '10.0.26100.0'
)

$ErrorActionPreference = 'Stop'
if (-not $OutputDirectory) {
    $OutputDirectory = Join-Path $PSScriptRoot 'out\cpuid-diagnostics'
}

$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
$vsRoot = & $vswhere -latest -products * -property installationPath
if (-not $vsRoot) {
    throw 'Visual Studio installation not found.'
}

$msvcRoot = Get-ChildItem -LiteralPath (Join-Path $vsRoot 'VC\Tools\MSVC') -Directory |
    Sort-Object Name -Descending |
    Select-Object -First 1 -ExpandProperty FullName
$kitsRoot = Join-Path ${env:ProgramFiles(x86)} 'Windows Kits\10'
$source = Join-Path $PSScriptRoot 'cpuid_diagnostic.cpp'
New-Item -ItemType Directory -Force -Path $OutputDirectory | Out-Null
$OutputDirectory = (Resolve-Path -LiteralPath $OutputDirectory).Path

foreach ($architecture in @('x86', 'x64')) {
    $cl = Join-Path $msvcRoot "bin\Hostx64\$architecture\cl.exe"
    if (-not (Test-Path -LiteralPath $cl)) {
        throw "Compiler not found: $cl"
    }

    $exe = Join-Path $OutputDirectory "cpuid_diagnostic-$architecture.exe"
    $pdb = Join-Path $OutputDirectory "cpuid_diagnostic-$architecture.pdb"
    $arguments = @(
        '/nologo', '/std:c++20', '/O2', '/MT', '/EHsc', '/Zi', '/Brepro',
        "/I$msvcRoot\include",
        "/I$kitsRoot\Include\$WindowsSdkVersion\ucrt",
        "/I$kitsRoot\Include\$WindowsSdkVersion\shared",
        "/I$kitsRoot\Include\$WindowsSdkVersion\um",
        "/Fd$pdb", "/Fe:$exe", $source, '/link', '/DEBUG:FULL', '/Brepro',
        "/PDB:$pdb",
        "/LIBPATH:$msvcRoot\lib\$architecture",
        "/LIBPATH:$kitsRoot\Lib\$WindowsSdkVersion\ucrt\$architecture",
        "/LIBPATH:$kitsRoot\Lib\$WindowsSdkVersion\um\$architecture"
    )
    & $cl @arguments
    if ($LASTEXITCODE -ne 0) {
        throw "CPUID diagnostic build failed for $architecture."
    }
}

Get-FileHash -Algorithm SHA256 -LiteralPath @(
    (Join-Path $OutputDirectory 'cpuid_diagnostic-x86.exe'),
    (Join-Path $OutputDirectory 'cpuid_diagnostic-x64.exe'),
    (Join-Path $OutputDirectory 'cpuid_diagnostic-x86.pdb'),
    (Join-Path $OutputDirectory 'cpuid_diagnostic-x64.pdb')
)
