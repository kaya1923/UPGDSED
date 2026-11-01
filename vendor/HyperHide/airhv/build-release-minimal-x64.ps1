[CmdletBinding()]
param(
    [string]$OutputDirectory = '',
    [string]$WindowsSdkVersion = '10.0.26100.0'
)

$ErrorActionPreference = 'Stop'

$airhvRoot = Join-Path $PSScriptRoot 'airhv'
if (-not $OutputDirectory) {
    $OutputDirectory = Join-Path $PSScriptRoot 'out\airhv-Release_Minimal-cpuid-cache'
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

$cl = Join-Path $msvcRoot 'bin\Hostx64\x64\cl.exe'
$ml64 = Join-Path $msvcRoot 'bin\Hostx64\x64\ml64.exe'
$link = Join-Path $msvcRoot 'bin\Hostx64\x64\link.exe'
$required = @(
    $cl,
    $ml64,
    $link,
    (Join-Path $kitsRoot "Include\$WindowsSdkVersion\km\ntddk.h"),
    (Join-Path $kitsRoot "Lib\$WindowsSdkVersion\km\x64\ntoskrnl.lib")
)
foreach ($path in $required) {
    if (-not (Test-Path -LiteralPath $path)) {
        throw "Required build input not found: $path"
    }
}

New-Item -ItemType Directory -Force -Path $OutputDirectory | Out-Null
$OutputDirectory = (Resolve-Path -LiteralPath $OutputDirectory).Path

Get-ChildItem -LiteralPath $OutputDirectory -File -ErrorAction SilentlyContinue |
    Where-Object { $_.Extension -in @('.obj', '.sys', '.pdb', '.map') } |
    Remove-Item -Force

Write-Output "Visual Studio: $vsRoot"
Write-Output "MSVC:          $msvcRoot"
Write-Output "Windows SDK:   $WindowsSdkVersion"
Write-Output "Source:        $airhvRoot"
Write-Output "Output:        $OutputDirectory"
Write-Output "Config:        Release_Minimal (_MINIMAL, CPUID cache §4.4)"

$compileArguments = @(
    '/nologo', '/c', '/kernel', '/std:c++17', '/O2', '/GL', '/GS-', '/GR-',
    '/EHs-c-', '/Zl', '/W3', '/Z7', '/Gy', '/Gw', '/Brepro',
    '/D_MINIMAL', '/D_AMD64_', '/DAMD64', '/D_WIN64',
    '/DNTDDI_VERSION=0x0A00000C', '/D_WIN32_WINNT=0x0A00', '/DWINVER=0x0A00',
    '/DUNICODE', '/D_UNICODE',
    ("/I$airhvRoot"),
    ("/I$msvcRoot\include"),
    ("/I$kitsRoot\Include\$WindowsSdkVersion\km"),
    ("/I$kitsRoot\Include\$WindowsSdkVersion\shared"),
    ("/I$kitsRoot\Include\$WindowsSdkVersion\ucrt")
)

$cppSources = @(
    'ept.cpp',
    'hypervisor_gateway.cpp',
    'hypervisor_routines.cpp',
    'invalidators.cpp',
    'log.cpp',
    'main.cpp',
    'poolmanager.cpp',
    'spinlock.cpp',
    'vmcs.cpp',
    'vmcall_handler.cpp',
    'vmexit_handler.cpp',
    'vmm.cpp'
)

$objects = @()
foreach ($name in $cppSources) {
    $source = Join-Path $airhvRoot $name
    $object = Join-Path $OutputDirectory (($name -replace '\.cpp$', '') + '.obj')
    & $cl @compileArguments "/Fo$object" $source
    if ($LASTEXITCODE -ne 0) {
        throw "Compilation failed: $name"
    }
    $objects += $object
}

$asmSources = @(
    @{ Rel = 'asm\vm_context.asm'; Out = 'vm_context.obj' },
    @{ Rel = 'asm\vm_intrin.asm'; Out = 'vm_intrin.obj' },
    @{ Rel = 'lde\lde64.asm'; Out = 'lde64.obj' }
)
foreach ($asm in $asmSources) {
    $source = Join-Path $airhvRoot $asm.Rel
    $object = Join-Path $OutputDirectory $asm.Out
    & $ml64 /nologo /c /Cx "/Fo$object" $source
    if ($LASTEXITCODE -ne 0) {
        throw "Assembly failed: $($asm.Rel)"
    }
    $objects += $object
}

$sys = Join-Path $OutputDirectory 'airhv.sys'
$pdb = Join-Path $OutputDirectory 'airhv.pdb'
$linkArguments = @(
    '/nologo', '/driver', '/subsystem:native', '/entry:DriverEntry', '/machine:x64',
    '/LTCG', '/OPT:REF', '/OPT:ICF', '/NODEFAULTLIB', '/MERGE:_TEXT=.text',
    '/SECTION:INIT,d', '/DEBUG:FULL', '/Brepro', '/PDBALTPATH:%_PDB%',
    "/PDB:$pdb",
    "/OUT:$sys",
    "/LIBPATH:$kitsRoot\Lib\$WindowsSdkVersion\km\x64"
) + $objects + @('ntoskrnl.lib', 'hal.lib', 'wmilib.lib', 'BufferOverflowK.lib')

& $link @linkArguments
if ($LASTEXITCODE -ne 0) {
    throw 'airhv link failed.'
}

Get-Item -LiteralPath $sys, $pdb | Select-Object FullName, Length, LastWriteTime
Get-FileHash -Algorithm SHA256 -LiteralPath $sys, $pdb
