# HANDOFF — Compile airhv + apply CPUID / RDTSC optimization

**Date:** 2026-10-30
**Target reader:** the *build / patch* agent (MSVC + WDK).
**Companion spec:** `docs/HANDOFF-CPUID-TSC-LATEX-SPEC-20261030.md` (the LaTeX writer's
"single source of truth"). **Do not rewrite that spec.** If you find a mismatch, fix the code
or this handoff, and flag the writer.
**Repo root for all paths below:**
`c:\Users\k\Desktop\mmf-bin-converter\tools\flex-x32dbg\local-session\UPGDSED-HyperHide-private\`

---

## 1. Goal (one sentence)

Ship a v5.2 airhv where the Flex `rdtsc; cpuid; rdtsc` bracket produces **one** VM exit
(CPUID) instead of **two** (RDTSC + CPUID), without changing the CPUID handler's observable
behavior (leaf 0, hypervisor vendor string, `hv_presence` gating all preserved).

## 2. Non-goals (do not touch)

- Do **not** untrap CPUID. In this build CPUID trapping is unconditional:
  `ia32\vmcs.h:39-72` (`__vmx_primary_processor_based_control`) has no "trap CPUID" bit, and
  `vmexit_handler.cpp:522-574` is the only path that runs when the guest executes CPUID.
- Do **not** enable `use_tsc_offsetting` (`vmcs.cpp:30`) or `use_tsc_scaling`
  (`vmcs.cpp:337`) — that is the *alternative* (spec §3c) and is **not** part of this handoff.
- Do **not** change the RDTSCP handler (`vmexit_handler.cpp:1151-1169`) or the RDTSC handler
  (`vmexit_handler.cpp:1209-1226`). If RDTSC is untrapped they are simply not reached from the
  bracket; leave them as the fallback for other guest paths.
- Do **not** change `vmm.cpp:200` (one-time `highest_basic_leaf` probe) or `common.h:129`.
- Do **not** rebuild the whole UPGDSED solution for the driver. `build.ps1` (repo root) only
  builds the UPGDSED tool; the driver is built from `vendor\HyperHide\airhv\airhv.sln`.

## 3. Ground truth (already verified against source — reproduce, don't re-derive)

Paths are relative to `vendor\HyperHide\airhv\airhv\` unless stated otherwise.

| Fact | File:line | Evidence (verbatim) |
|---|---|---|
| RDTSC trap toggle (build-gated) | `vmcs.cpp:60-64` | `#ifdef _MINIMAL` → `rdtsc_exiting=false;` / `#else` → `=true;` / `#endif` |
| TSC offset off | `vmcs.cpp:30` | `primary_controls.use_tsc_offsetting = false;` |
| TSC scaling off | `vmcs.cpp:337` | `secondary_controls.use_tsc_scaling = false;` |
| CPUID handler | `vmexit_handler.cpp:522-574` | `vmexit_cpuid_handler()` |
| — leaf-0 fallback | `vmexit_handler.cpp:529` | `__cpuidex((int*)&cpuid_reg.eax, g_vmm_context->highest_basic_leaf, 0);` |
| — leaf dispatch | `vmexit_handler.cpp:535` | `switch (vcpu->vmexit_info.guest_registers->rax)` |
| — HV vendor leaf | `vmexit_handler.cpp:541-549` | `CPUID_HV_VENDOR_AND_MAX_FUNCTIONS` → `ebx='hria'` |
| — `hv_presence` gating | `vmexit_handler.cpp:538,542,552,563` | all four special-case branches test `g_vmm_context->hv_presence` |
| RDTSC handler | `vmexit_handler.cpp:1209-1226` | `__int64 tsc = __rdtsc();` at line 1220 |
| RDTSCP handler | `vmexit_handler.cpp:1151-1169` | `__int64 tsc = __rdtscp(&processor_id);` at line 1163 |
| `highest_basic_leaf` field | `common.h:129` | `unsigned __int32 highest_basic_leaf;` |
| `highest_basic_leaf` init | `vmm.cpp:200` | `g_vmm_context->highest_basic_leaf = cpuid_reg.eax;` |
| No CPUID trap bit | `ia32\vmcs.h:39-72` | union `__vmx_primary_processor_based_control` |
| Shipped config is `_MINIMAL` | `HYPERHIDE_CRASH_ANALYSIS.md:35`; `nextsteps.md:392`; `docs/HANDOFF_HYPERHIDE_V5_V6.md:214` | "airhv is built as `Release_Minimal`" |

> **Note on the LaTeX spec:** its §2 table says "TSC scaling off | `vmcs.cpp:337` |
> `primary_controls.use_tsc_scaling`". The actual source says `secondary_controls.use_tsc_scaling`.
> Report this to the writer; **do not** "fix" it in the spec yourself.
## 4. Build (only what's needed for this task)

### 4.1 Prerequisites
- Windows 10/11 dev box with Visual Studio 2019/2022 + C++ workload.
- WDK matching the Win10 19045 SDK (`10.0.19041.0`) already used by the repo
  (`docs/HANDOFF_HYPERHIDE_V5_V6.md:211-215`).
- `msbuild` on PATH (Developer Command Prompt / VS Dev Shell).

### 4.2 Command
```
cd c:\Users\k\Desktop\mmf-bin-converter\tools\flex-x32dbg\local-session\UPGDSED-HyperHide-private
msbuild vendor\HyperHide\airhv\airhv.sln /p:Configuration=Release_Minimal /p:Platform=x64 /t:Rebuild /m /v:minimal
```

Notes:
- **Configuration must be `Release_Minimal`** (not `Release`). `Release_Minimal` defines
  `_MINIMAL`, which is the switch at `vmcs.cpp:60-64` that leaves RDTSC/RDTSCP untrapped —
  this *is* the 3a optimization from the spec, delivered by the build flag.
- If the solution has no `Release_Minimal` config, add it by copying the `Release` project
  config and adding `_MINIMAL` to `PreprocessorDefinitions`. Do **not** edit `Release`.
- Expected artifact: `vendor\HyperHide\airhv\airhv\Release_Minimal\airhv.sys`
  (adjust sub-path if the vcxproj redirects `OutDir`; read the `.vcxproj` to confirm).

### 4.3 Optional code patch (only if you must ship a non-`_MINIMAL` build)

If the remote agent is required to ship a `Release` (full) build, apply **exactly** this one
change in `vendor\HyperHide\airhv\airhv\vmcs.cpp:60-64`:

```cpp
// BEFORE
#ifdef _MINIMAL
	primary_controls.rdtsc_exiting = false;
#else
	primary_controls.rdtsc_exiting = true;
#endif

// AFTER
primary_controls.rdtsc_exiting = false;   // untrap RDTSC/RDTSCP (see docs/HANDOFF-CPUID-TSC-LATEX-SPEC-20261030.md §3a)
```

Do **not** touch any other line in `vmcs.cpp` as part of this handoff. In particular:
- leave `use_tsc_offsetting` (line 30) and `use_tsc_scaling` (line 337) as `false`;
- leave the other `#ifdef _MINIMAL` blocks (CR3, INVLPG, MWAIT, RDPMC, MOV-to-DR,
  MSR bitmaps, …) unchanged unless explicitly asked — that's a different decision; call it out.

### 4.4 Optional CPUID caching (spec §3b, do this *only if* 3a alone is not enough)

The CPUID handler calls `__cpuidex` on **every** leaf (lines 529 / 532). For the two leaves
the Flex bracket actually probes (leaf 0 and the HV vendor leaf at line 541), a tiny cache
eliminates the redundant host-CPUID execution on repeated queries.

Rules (all must hold):
1. Add a cache in `common.h` next to `highest_basic_leaf` (line 129):
   `struct { unsigned __int32 eax, ebx, ecx, edx; } cpuid_leaf0_cache, cpuid_hvvendor_cache;`
   (exact shape is up to you; keep it in `g_vmm_context`).
2. Populate **once** from the one-time host CPUID at `vmm.cpp:200` (next to the
   `highest_basic_leaf` init). **Do not** re-probe on every guest CPUID.
3. In `vmexit_cpuid_handler()`:
   - leaf 0 (RAX in `[0,0x7fffffff]`): serve from cache, then apply the same `switch` at line
     535 (leaf 0 has no special-case branch → cache is safe).
   - `CPUID_HV_VENDOR_AND_MAX_FUNCTIONS`: serve from cache, then apply the `hv_presence`
     gating at lines 541-549 (cache stores the *base* value; gate applied on read so
     `hv_presence` changes are honored without invalidation).
   - **Every other leaf:** keep the current `__cpuidex` path. Do **not** cache leaves that
     have a special-case branch unless you've verified its `hv_presence`/`rcx` dependencies
     (e.g. `CPUID_EXTENDED_FEATURES` at 562-565 uses `rcx`).
4. Invalidation: if `g_vmm_context->hv_presence` is ever reassigned (search the codebase for
   `hv_presence =`), invalidate both caches. If it's set exactly once at init, no invalidation
   code is needed — say so in the summary.
5. **Do not** change `adjust_rip(vcpu);` at line 573 or the register writes at 568-571.

## 5. Deploy (after build)

1. Back up the current driver:
   `bin\HyperHide\HyperHide-Win10-19045-x64-v5.2-kusd-lifecycle\airhv.sys`
   → rename to `airhv.sys.bak.<yyyymmdd>` (do **not** delete).
2. Copy the new `airhv.sys` over it (same path, same file name — `SOURCE-IDENTITY.txt` in
   that folder is the contract; update it *after* a successful run, not before).
3. **Do not** touch the known-good ZIPs (`v5.2-kusd-lifecycle.zip`,
   `v5.2-kusd-lifecycle-final.zip`, and `v5.2-kusd-lifecycle-vpid-fixed.zip` in the same
   folder). They are the rollback points.

## 6. Verification (must be reported back)

| # | Check | Expected |
|---|---|---|
| 1 | `airhv.sys` exists at the §4.2 output path and is newer than the source files you patched. | ok |
| 2 | Load the driver on the target host (`sc create` / the existing HyperHide install flow). | no BSOD; driver starts |
| 3 | Run the Flex detector and record the `rdtsc; cpuid; rdtsc` bracket delta (raw + delta). | delta <= pre-patch baseline, not higher |
| 4 | Count VM exits during the bracket, classified by reason. | `EXIT_REASON_CPUID` = 1, RDTSC exit = 0 |
| 5 | CPUID leaf 0 (EAX/EBX/ECX/EDX) and `0x40000000` (HV vendor) return the **same** values as the pre-patch build. | identical |
| 6 | `hv_presence` toggle (if your build has a toggle path) still flips the leaf-1 hypervisor bit (`vmexit_handler.cpp:538`) and the HV-vendor leaf contents (`:542-548`). | both states correct |

Capture (mirrors spec §4):
- raw + computed bracket delta (both sides, pre and post);
- per-exit-reason counts;
- guest RAX at RDTSC handler entry vs. host `__rdtsc()` at that instant (only relevant if
  you also patched the RDTSC path — otherwise note "handler not reached");
- `highest_basic_leaf` value and the leaf-0 / HV-vendor results.

## 7. Risks / gotchas

- **`_MINIMAL` changes more than RDTSC.** `vmcs.cpp` uses `#ifdef _MINIMAL` for several other
  controls (CR3, INVLPG, MWAIT, RDPMC, MOV-to-DR, MSR bitmaps, …). Building with
  `Release_Minimal` flips **all** of them. If any of those are load-bearing for your target
  workload, say so before shipping. This is exactly why `HYPERHIDE_CRASH_ANALYSIS.md:35` says
  the bundled driver is already `_MINIMAL` — so the default here matches the known-good baseline.
- **Don't invent a "trap CPUID off" bit.** The primary-control union at `ia32\vmcs.h:39-72`
  has no such bit. Anyone claiming they "disabled CPUID trapping" is wrong.
- **RDTSC handler still exists** at `vmexit_handler.cpp:1209-1226` and returns the raw host
  TSC (`__rdtsc()` at line 1220). It is now just a fallback. Do **not** change it as part of
  this handoff; changing it to a synthetic value is spec §3c (a separate decision).
- **The Flex bracket is only *cheaper*, not *invisible*.** After this patch it is one CPUID
  exit + one CPUID-handler invocation. Any detector that counts CPUID exits will still see it.
  That's by design (spec §5: "the fix is to cheapen and de-redundantize, not to untrap CPUID").
- **Encoding:** `vmexit_handler.cpp` contains a `?` in a comment at line 1154
  ("processor?s time-stamp counter"). Leave it alone unless doing a clean re-encode.

## 8. Deliverables (what you send back)

1. A 5-15 line summary: which config you built, which files you touched (or none), and the
   §6 verification results (numbers, not adjectives).
2. The new `airhv.sys` in the §5 path, with the old one kept as `.bak.<date>`.
3. A list of every file changed (path + one-line description) — must be **zero** if you only
   built, **one** if you applied the §4.3 patch, **two or three** if you applied §4.4.
4. The Flex output (raw + delta) and the per-exit-reason count, so the writer can quote them
   verbatim.
5. Any discrepancy you found vs. §3 (e.g. the `primary_controls` vs `secondary_controls`
   `use_tsc_scaling` mismatch) — one bullet per discrepancy.

## 9. Out of scope (do **not** do these in this handoff)

- Enable `use_tsc_offsetting` (spec §3c).
- Modify the RDTSC / RDTSCP handlers to compute a synthetic TSC.
- Add a new VM-exit handler or change the exit-reason dispatch.
- Rebuild UPGDSED, its plugins (`.dp64` / `.dp32`), or any non-airhv target.
- Touch `invalidators.cpp`, `vmm.cpp` (except the §4.4 cache init if you take it on), or any
  `.h` under `ia32\` other than reading them.
- Change `SOURCE-IDENTITY.txt` until verification (§6) is green.

---
*End of handoff. If any step is ambiguous, ask before patching — the spec is the tiebreaker.*

