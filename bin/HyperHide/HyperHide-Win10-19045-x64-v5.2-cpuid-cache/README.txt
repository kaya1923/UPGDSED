HyperHide Win10 19045 x64 — v5.2 HyperHideDrv + airhv CPUID-cache

Was geändert
- Nur airhv.sys: einmaliger Host-CPUID-Cache für Leaf 0 und den versteckten
  0x40000000..0x4FFFFFFF-Pfad (HANDOFF-CPUID-TSC-OPTIMIZE-DRIVER §4.4 / 3b).
- Release_Minimal (_MINIMAL): RDTSC bleibt ungetrappt (3a war schon so).
- HyperHideDrv, Plugin, Offsets, NtQueryObject-Fix: unverändert aus v5.2.

Was nicht geändert
- Kein TSC-Offset/Scaling, kein CPUID-Untrap, keine RDTSC-Handler-Änderung.

Deploy (HP-Bench only)
1. Backup vorhandener Treiber.
2. preflight.ps1 → install.ps1 → start.ps1 -AirhvOnly zuerst.
3. Timing/Exit-Counts am rdtsc;cpuid;rdtsc-Bracket vs. Baseline C4F55AD0…
4. Bei Problemen: airhv.sys.bak.20261031 bzw. v5.2-kusd-lifecycle.zip.

Nicht auf dem Build-PC laden.
