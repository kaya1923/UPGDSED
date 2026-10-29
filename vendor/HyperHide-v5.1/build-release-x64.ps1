[CmdletBinding()]
param(
    [string]$OutputDirectory = '',
    [string]$WindowsSdkVersion = '10.0.26100.0'
)

$ErrorActionPreference = 'Stop'

if (-not $OutputDirectory) {
    $OutputDirectory = Join-Path $PSScriptRoot 'out\HyperHideDrv-v5.1'
}

$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
if (-not (Test-Path -LiteralPath $vswhere)) {
    throw "vswhere.exe not found: $vswhere"
}

$vsRoot = & $vswhere -latest -products * -property installationPath
if (-not $vsRoot) {
    throw 'Visual Studio installation not found.'
}

$msvcRoot = Get-ChildItem -LiteralPath (Join-Path $vsRoot 'VC\Tools\MSVC') -Directory |
    Sort-Object Name -Descending |
    Select-Object -First 1 -ExpandProperty FullName
$kitsRoot = Join-Path ${env:ProgramFiles(x86)} 'Windows Kits\10'
$sourceRoot = Join-Path $PSScriptRoot 'HyperHideDrv'

$cl = Join-Path $msvcRoot 'bin\Hostx64\x64\cl.exe'
$ml64 = Join-Path $msvcRoot 'bin\Hostx64\x64\ml64.exe'
$link = Join-Path $msvcRoot 'bin\Hostx64\x64\link.exe'
$required = @(
    $cl,
    $ml64,
    $link,
    (Join-Path $kitsRoot "Include\$WindowsSdkVersion\km\ntifs.h"),
    (Join-Path $kitsRoot "Lib\$WindowsSdkVersion\km\x64\ntoskrnl.lib")
)
foreach ($path in $required) {
    if (-not (Test-Path -LiteralPath $path)) {
        throw "Required build input not found: $path"
    }
}

New-Item -ItemType Directory -Force -Path $OutputDirectory | Out-Null
$OutputDirectory = (Resolve-Path -LiteralPath $OutputDirectory).Path

# Remove only artifacts produced by this script. /Z7 avoids a shared compiler
# PDB whose accumulated state otherwise changes the LTCG driver image.
Get-ChildItem -LiteralPath $OutputDirectory -File -ErrorAction SilentlyContinue |
    Where-Object { $_.Extension -in @('.obj', '.sys', '.pdb', '.exe') } |
    Remove-Item -Force

Write-Output "Visual Studio: $vsRoot"
Write-Output "MSVC:          $msvcRoot"
Write-Output "Windows SDK:   $WindowsSdkVersion"
Write-Output "Source:        $sourceRoot"
Write-Output "Output:        $OutputDirectory"

$compileArguments = @(
    '/nologo', '/c', '/kernel', '/std:c++20', '/O2', '/GL', '/GS-', '/GR-',
    '/EHs-c-', '/Zl', '/W3', '/Z7', '/Gy', '/Gw', '/Brepro',
    '/D_AMD64_', '/DAMD64', '/D_WIN64',
    '/DNTDDI_VERSION=0x06010000', '/D_WIN32_WINNT=0x0601', '/DWINVER=0x0601',
    '/DUNICODE', '/D_UNICODE',
    ("/I$sourceRoot"),
    ("/I$msvcRoot\include"),
    ("/I$kitsRoot\Include\$WindowsSdkVersion\km"),
    ("/I$kitsRoot\Include\$WindowsSdkVersion\shared"),
    ("/I$kitsRoot\Include\$WindowsSdkVersion\ucrt")
)

$objects = @()
foreach ($source in Get-ChildItem -LiteralPath $sourceRoot -Filter '*.cpp' | Sort-Object Name) {
    $object = Join-Path $OutputDirectory ($source.BaseName + '.obj')
    & $cl @compileArguments "/Fo$object" $source.FullName
    if ($LASTEXITCODE -ne 0) {
        throw "Compilation failed: $($source.Name)"
    }
    $objects += $object
}

$assemblerObject = Join-Path $OutputDirectory 'vmintrin.obj'
& $ml64 /nologo /c /Fo $assemblerObject (Join-Path $sourceRoot 'vmintrin.asm')
if ($LASTEXITCODE -ne 0) {
    throw 'Assembly failed: vmintrin.asm'
}
$objects += $assemblerObject

$linkArguments = @(
    '/nologo', '/driver', '/subsystem:native', '/entry:DriverEntry', '/machine:x64',
    '/LTCG', '/OPT:REF', '/OPT:ICF', '/NODEFAULTLIB', '/MERGE:_TEXT=.text',
    '/SECTION:INIT,d', '/DEBUG:FULL', '/Brepro', '/PDBALTPATH:%_PDB%',
    "/PDB:$OutputDirectory\HyperHideDrv.pdb",
    "/OUT:$OutputDirectory\HyperHideDrv.sys",
    "/LIBPATH:$kitsRoot\Lib\$WindowsSdkVersion\km\x64"
) + $objects + @('ntoskrnl.lib', 'hal.lib', 'wmilib.lib', 'BufferOverflowK.lib')

& $link @linkArguments
if ($LASTEXITCODE -ne 0) {
    throw 'HyperHideDrv link failed.'
}

$testSource = Join-Path $PSScriptRoot 'tests\ntqueryobject_safety_tests.cpp'
$testExe = Join-Path $OutputDirectory 'ntqueryobject_safety_tests.exe'
$testArguments = @(
    '/nologo', '/std:c++20', '/EHsc', '/Brepro',
    "/I$msvcRoot\include",
    "/I$kitsRoot\Include\$WindowsSdkVersion\ucrt",
    "/I$kitsRoot\Include\$WindowsSdkVersion\shared",
    "/I$kitsRoot\Include\$WindowsSdkVersion\um",
    "/Fe:$testExe", $testSource, '/link',
    "/LIBPATH:$msvcRoot\lib\x64",
    "/LIBPATH:$kitsRoot\Lib\$WindowsSdkVersion\ucrt\x64",
    "/LIBPATH:$kitsRoot\Lib\$WindowsSdkVersion\um\x64"
)
& $cl @testArguments
if ($LASTEXITCODE -ne 0) {
    throw 'Offline regression test build failed.'
}
& $testExe
if ($LASTEXITCODE -ne 0) {
    throw 'Offline regression tests failed.'
}

$artifacts = @(
    (Join-Path $OutputDirectory 'HyperHideDrv.sys'),
    (Join-Path $OutputDirectory 'HyperHideDrv.pdb')
)
Get-FileHash -Algorithm SHA256 -LiteralPath $artifacts
