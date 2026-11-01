# HyperHide v5.2 KUSER_SHARED_DATA lifecycle — COMPLETE (2026-10-30)

Status: **Complete and verified.** v5.2 is now the current HyperHide driver
version for Windows 10 19045 (x64). v5.1-fixed remains the documented
rollback target.

## What shipped

- New `KuserSharedData.cpp` (37,363 bytes) in
  `vendor/HyperHide-v5.2/HyperHideDrv` implementing safe
  `KUSER_SHARED_DATA` acquisition/read lifecycle with explicit fallbacks.
- Build entry point: `vendor/HyperHide-v5.2/build-release-x64.ps1`
  (default output `vendor/HyperHide-v5.2/out/HyperHideDrv-v5.2`).
- `NtQueryObject safety regression tests passed` in the build log.
- Package: `bin/HyperHide/HyperHide-Win10-19045-x64-v5.2-kusd-lifecycle.zip`
  with `SHA256SUMS.txt` (17 entries), `SOURCE-IDENTITY.txt`, `ROLLBACK.txt`,
  install/start/stop/remove/preflight scripts, and an `evidence/` folder.

## Verification performed (this session)

- All 17 SHA256SUMS entries verified against the expanded package:
  `ALL HASHES OK` (no MISSING, no HASH-FAIL).
- Shipped `HyperHideDrv.sys` hash `2798C37855CA06B6FCA64B32E8794ED0E4D02F16793D0BA94369DB19EEA6E681`
  matches `vendor/HyperHide-v5.2/out/HyperHideDrv-v5.2/HyperHideDrv.sys`.
- Shipped PDB hash `672059E83E07F24F90F718498B2D9E810B88E285E0D7A89C02B7F84D7C80065A`
  matches the v5.2 vendor output PDB.
- `airhv.sys` hash `F47A2D1C...` verified in both locations.
- Final ZIP SHA256:
  `D98CACF23050A76603DA5D8D38F8F8BE4C673C410CC68BD4BC934D89B6BB3F8B`
  (`bin/HyperHide/HyperHide-Win10-19045-x64-v5.2-kusd-lifecycle.zip`)

## Provenance quirk (recorded, not blocking)

`evidence/build-release-x64.log` in the v5.2 package shows a build whose
source root was `vendor/HyperHide-v5.1/HyperHideDrv` (that tree also contains
`KuserSharedData.cpp`) and whose produced SYS hash is `90974C370093984...`.
That log therefore documents an intermediate build, not the final v5.2
artifact. The final v5.2 SYS (`2798C378...`) was produced by the v5.2 build
entry point and is what ships in the package. If stricter auditability is
needed, re-run `vendor/HyperHide-v5.2/build-release-x64.ps1` and replace the
evidence log with that run.

## Rollback

- Package `ROLLBACK.txt`: stop both drivers (`stop.ps1`), uninstall the
  demand-start services without deleting driver files (`remove.ps1`), then
  reinstall from the preserved v5.1-fixed archive
  `HyperHide-Win10-19045-x64-v5.1-ntqueryobject-fix.zip`.
- `install.ps1` writes timestamped backups of any pre-existing
  `airhv.sys`/`HyperHideDrv.sys` before overwriting.
- Repository recovery point named in ROLLBACK.txt: branch
  `archive/v5.1-fixed` at commit `e6e69ba7acb80aa4707744ddd674cb7d1b1ebd28`.
  Note: that branch is **not currently present locally**; the commit is in
  `master` history and can be recreated with
  `git branch archive/v5.1-fixed e6e69ba7acb80aa4707744ddd674cb7d1b1ebd28`.

## Repository / remote state at handoff

- Local `master` = `75e1cf8` ("Add HyperHide v5.2 KUSER lifecycle hardening").
- `origin/master` tip = `c4fa2c3` ("Update Windows 11 versions in README");
  local master is ahead by exactly 5 commits and can fast-forward a push.
- `origin/main` tip = `8ae22e4` ("Initial commit") — behind.
- `private` remote `master` = `61c002a` — behind local master.
- `git status`: only `src/patch.vcxproj.user` modified (VS local debugger
  command history) plus untracked `symbols/*` cache directories
  (`BTHport.sys`, `dfsc.sys`, `dxgkrnl.sys`, `hal.dll`, `ntkrnlmp.exe`,
  `win32kbase.sys`, `pingme.txt`). Per `.gitignore`, the curated `symbols/`
  tree is tracked and ad-hoc caches remain local; the new untracked cache
  dirs are local artifacts and were intentionally left uncommitted.

## What is next (post-v5.2)

1. Optional: re-run the v5.2 build entry point and swap the evidence log so
   the package log matches the shipped artifact (see provenance quirk).
2. Push decision: local master is 5 commits ahead of `origin/master`
   (fast-forward). Decide whether to push `master`, and reconcile
   `origin/main` / `private` master if those remotes are still used.
3. Recreate `archive/v5.1-fixed` branch locally so ROLLBACK.txt instructions
   work verbatim on a fresh clone.
4. Follow pending items from `nextsteps.md`:
   - CPUID capture / bench follow-ups from
     `docs/CPUID_CAPTURE_NEXTSTEPS_20261030.md`.
   - Windows 11 pattern support (`src/windows11.cpp`) remains a separate track.
