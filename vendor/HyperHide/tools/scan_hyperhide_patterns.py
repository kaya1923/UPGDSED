import os
from pathlib import Path

import pefile
from capstone import Cs, CS_ARCH_X86, CS_MODE_64


def section_bytes(pe: pefile.PE, name: bytes):
    for section in pe.sections:
        if section.Name.rstrip(b"\0") == name:
            return section.VirtualAddress, section.get_data()
    raise RuntimeError(f"section {name!r} not found")


def find_masked(data: bytes, pattern: bytes, mask: str):
    hits = []
    limit = len(data) - len(pattern) + 1
    for offset in range(max(limit, 0)):
        if all(m == "?" or data[offset + index] == pattern[index]
               for index, m in enumerate(mask)):
            hits.append(offset)
    return hits


def report(image: Path, section: bytes, label: str, pattern: bytes, mask: str):
    pe = pefile.PE(str(image), fast_load=False)
    rva, data = section_bytes(pe, section)
    hits = find_masked(data, pattern, mask)
    locations = ", ".join(f"0x{rva + hit:X}" for hit in hits) or "none"
    print(f"{label}: count={len(hits)} rva={locations}")


windows = Path(os.environ["SystemRoot"])
kernel = windows / "System32" / "ntoskrnl.exe"

kernel_pe = pefile.PE(str(kernel), fast_load=False)
ki_dispatch_rva = 0x305BE0  # Microsoft PDB for ntoskrnl.exe 10.0.19041.7802
ki_dispatch_bytes = kernel_pe.get_data(ki_dispatch_rva, 96)
print(f"KiDispatchException PDB RVA 0x{ki_dispatch_rva:X}: {ki_dispatch_bytes[:32].hex(' ')}")
for instruction in list(Cs(CS_ARCH_X86, CS_MODE_64).disasm(ki_dispatch_bytes, ki_dispatch_rva))[:12]:
    print(f"  0x{instruction.address:X}: {instruction.mnemonic} {instruction.op_str}")

report(
    kernel,
    b".text",
    "KeServiceDescriptorTableShadow",
    bytes.fromhex("4C 8D 15 00 00 00 00 4C 8D 1D 00 00 00 00 F7"),
    "xxx????xxx????x",
)
report(
    kernel,
    b".text",
    "MiGetPteAddress",
    bytes.fromhex(
        "48 C1 E9 00 48 B8 00 00 00 00 00 00 00 00 48 23 C8 "
        "48 B8 00 00 00 00 00 00 00 00 48 03 C1 C3"
    ),
    "xxx?xx????????xxxxx????????xxxx",
)
report(
    kernel,
    b".text",
    "KiDispatchException callsite",
    bytes.fromhex("8B 00 50 00 8B 00 58 48 8D 4D 00 E8 00 00 00 00 8B 55"),
    "x?x?x?xxxx?x???xxx",
)
report(
    kernel,
    b".text",
    "KiDispatchException direct",
    bytes.fromhex(
        "40 55 53 56 41 54 41 56 41 57 48 81 EC 88 01 00 00 "
        "48 8D 6C 24 30 48 8B 05 00 00 00 00 48 33 C5"
    ),
    "xxxxxxxxxxxxxxxxxxxxxxxxx????xxx",
)

for dll_name, exports in {
    "ntdll.dll": [
        "NtContinueEx", "NtSetInformationThread", "NtQueryInformationProcess",
        "NtQueryObject", "NtSystemDebugControl", "NtSetContextThread",
        "NtQuerySystemInformation", "NtGetContextThread", "NtClose",
        "NtQueryInformationThread", "NtCreateThreadEx", "NtCreateFile",
        "NtCreateProcessEx", "NtYieldExecution", "NtQuerySystemTime",
        "NtQueryPerformanceCounter", "NtQueryInformationJobObject",
        "NtCreateUserProcess", "NtGetNextProcess", "NtOpenProcess",
        "NtOpenThread", "NtSetInformationProcess",
    ],
    "win32u.dll": [
        "NtUserBuildHwndList", "NtUserFindWindowEx", "NtUserQueryWindow",
        "NtUserGetForegroundWindow", "NtUserGetThreadState",
    ],
}.items():
    image = windows / "System32" / dll_name
    pe = pefile.PE(str(image), fast_load=False)
    names = {
        symbol.name.decode("ascii")
        for symbol in pe.DIRECTORY_ENTRY_EXPORT.symbols
        if symbol.name
    }
    missing = [name for name in exports if name not in names]
    print(f"{dll_name} exports: missing={missing or 'none'}")
