# Build report — airhv CPUID cache (§4.4 / 3b)

Date: 2026-10-31 (build PC). No driver loaded here.

## Summary

- Config: `Release_Minimal` (`_MINIMAL`) via
  `vendor/HyperHide/airhv/build-release-minimal-x64.ps1`
  (WDK MSBuild toolset not registered in VS 18 → same manual path as v5.1).
- Touched sources: `common.h`, `vmm.cpp`, `vmexit_handler.cpp`.
- Optional §4.3 (`rdtsc_exiting=false` in non-Minimal): **not applied** —
  shipped baseline already `_MINIMAL` / 3a.
- `hv_presence` toggles in `vmcall_handler.cpp`: **no cache invalidation** —
  caches hold host CPUID bases; gating still applied on every exit.

## Artifacts

| Item | SHA-256 |
|------|---------|
| `airhv.sys` (new) | `494166ED8CD327447EABBFCC28A2C2F64ED91C646BE6D37ED896DCD0FE09F9DA` |
| Package ZIP | `13E956A999F707AE9B387F5BE09C3569B928B2F09224C45FADE269918986D0BC` (`HyperHide-Win10-19045-x64-v5.2-cpuid-cache.zip`) |
| Prior airhv backup | `.../v5.2-kusd-lifecycle/airhv.sys.bak.20261031` (= `C4F55AD0…`) |
| Known-good v5.2 `airhv.sys` | **unchanged** `C4F55AD0…` |
| Known-good ZIPs | **not modified** |

HyperHideDrv in the new package is the unchanged v5.2 image.

## Verification on build PC

| # | Check | Result |
|---|-------|--------|
| 1 | Newer `airhv.sys` at build out path | ok (65024 bytes) |
| 2 | Load driver | **not run** (bench only) |
| 3–4 | Flex bracket delta / exit counts | **pending bench** |
| 5 | Leaf 0 / `0x40000000` register identity | semantic-preserving cache; bench confirm |
| 6 | `hv_presence` gating | still in switch after cache read |

## Discrepancies vs handoff/spec

- LaTeX/driver handoff §3a framed as the main ship step; baseline airhv was
  already `Release_Minimal` → only §4.4 implemented.
- Spec table attributes `use_tsc_scaling` to `primary_controls`; source uses
  `secondary_controls` (`vmcs.cpp` ~337).
- Handoff leaf-0 wording `[0,0x7fffffff]` would wrongly cache all basic leaves;
  implementation caches **leaf 0 / subleaf 0 only**.
