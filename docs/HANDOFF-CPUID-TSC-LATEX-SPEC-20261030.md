# SPEC — CPUID-leak fix + TSC capture (for the LaTeX writer)

**Date:** 2026-10-30
**Status:** spec for the *writer* agent (LaTeX). **Do not implement code.** Produce a precise,
implementation-ready LaTeX section.
**Goal for the LaTeX doc:** a short, correct technical description of (1) the CPUID-leak fix and
(2) the TSC capture, grounded in the actual HyperHide/airhv code, with the exact code points cited
so the writer can reference them verbatim.

This file is the **single source of truth** for what the LaTeX must contain. Every claim below is
already verified against the code — the writer must reproduce it faithfully, not invent.

---

## 0. Scope and ground rules (writer)

- Audience: a reader who knows x86/VT-x basics but has not read airhv. Keep it self-contained.
- **Do not** write code for the writer to invent. Cite the existing code (file:line) and describe
  the *intended* mechanism in words. Small pseudocode snippets are OK only to clarify a mechanism.
- No TODO/placeholder text. No invented function names, constants, or bit positions.
- Keep it tight. This is a focused technical addendum, not a tutorial.
- Language: English, present tense, precise. No marketing.

---

## 1. Problem (state exactly this)

Flex (a 32-bit CPUID-gadget detector) probes the host via a `rdtsc; cpuid; rdtsc` bracket.
The two `rdtsc`s read the TSC immediately before/after a `cpuid`; the delta reveals VM-exit
overhead. In HyperHide (airhv) the bracket currently forces **two** VM exits, not one, because
CPUID is *always* trapped **and** RDTSC is *also* trapped:

- CPUID trapping is unconditional in this build: the primary-control struct
  (`airhv\ia32\vmcs.h:39-60`) has **no** "trap CPUID" bit, so CPUID always causes a VM exit.
- RDTSC is *additionally* trapped: `set_primary_controls()` sets
  `primary_controls.rdtsc_exiting = true;` (`airhv\vmcs.cpp:60-64`).

So the leak is a **two-exit** leak (RDTSC exit + CPUID exit + RDTSC exit). The RDTSC trap is
currently pure overhead: the handler returns the raw host TSC (see §2), so trapping RDTSC adds
cost without changing the result.

---

## 2. Verified facts to cite (use these file:line anchors verbatim)

All paths relative to the repo root
`tools\flex-x32dbg\local-session\UPGDSED-HyperHide-private\vendor\HyperHide\airhv\airhv`.

| Fact | Where | Evidence |
|---|---|---|
| CPUID exit handler | `vmexit_handler.cpp:522-574` | `vmexit_cpuid_handler()` |
| — reads `highest_basic_leaf` into EAX | `vmexit_handler.cpp:529` | `__cpuidex((int*)&cpuid_reg.eax, g_vmm_context->highest_basic_leaf, 0);` |
| — reads ECX/EDX | `vmexit_handler.cpp:537-538` | from `g_vmm_context->cpuid_reg` |
| — `highest_basic_leaf` type | `common.h:129` | `unsigned __int32 highest_basic_leaf;` |
| — `highest_basic_leaf` initialized once | `vmm.cpp:200` | `g_vmm_context->highest_basic_leaf = cpuid_reg.eax;` (from a one-time host CPUID) |
| RDTSC exit handler | `vmexit_handler.cpp:1209-1226` | `vmexit_rdtsc_handler()` |
| — **returns raw host TSC** | `vmexit_handler.cpp:1213` | `__int64 tsc = __rdtsc();` then `vmwrite(GUEST_RAX, tsc)` |
| RDTSCP exit handler | `vmexit_handler.cpp:1151-1169` | `vmexit_rdtscp_handler()`; `__int64 tsc = __rdtscp();` (line 1155) |
| RDTSC trap enabled | `vmcs.cpp:60-64` | `primary_controls.rdtsc_exiting = true;` (non-`_MINIMAL`) |
| TSC offsetting **off** | `vmcs.cpp:30` | `primary_controls.use_tsc_offsetting = false;` (both builds) |
| TSC scaling **off** | `vmcs.cpp:337` | `primary_controls.use_tsc_scaling = false;` (non-`_MINIMAL`) |
| `_MINIMAL` mode leaves RDTSC **un**trapped | `vmcs.cpp:61-64` | `#ifndef _MINIMAL … #else` branch does not set `rdtsc_exiting` |
| Primary-control bit fields | `ia32\vmcs.h:46,53` | `use_tsc_offsetting : 1;` … `rdtsc_exiting : 1;` |
| VMCS TSC fields | `vmcs_encodings.h:168,185` | `CONTROL_TSC_OFFSET`, `CONTROL_TSC_MULTIPLIER` |

**Conclusion for the reader:** CPUID trapping is the leak; RDTSC trapping is a *redundant* second
exit that the current handler does not even need (it just forwards the host TSC).

---

## 3. The CPUID-leak fix — spec for the LaTeX

Describe the **intended** mechanism in three parts. Present it as the design, citing the existing
handler as the reference point the design modifies/extends.

### 3a. Remove the redundant RDTSC exit (preferred, minimal)

- **Change:** set `primary_controls.rdtsc_exiting = false;` in `set_primary_controls()`
  (`vmcs.cpp:60-64`), i.e. leave RDTSC untrapped (as `_MINIMAL` already does).
- **Effect:** the bracket's two `rdtsc`s then read the (offsetted) guest TSC directly with **no
  VM exit**; only the `cpuid` exits. This turns a two-exit leak into a one-exit leak.
- **Caveat to state honestly:** this removes a *second* exit but does not by itself make the
  bracket invisible; the CPUID exit remains. Pair with 3b.

### 3b. Cheapen the CPUID exit path for the leak probe

- **Observation (cite):** `vmexit_cpuid_handler()` (`vmexit_handler.cpp:522-574`) currently calls
  a **real** `__cpuidex` on the host for every guest CPUID (`vmexit_handler.cpp:529`), then writes
  ECX/EDX from the cached `g_vmm_context->cpuid_reg` (`:537-538`). The per-exit host CPUID is the
  dominant cost and the thing Flex is timing.
- **Intended design (state as intent, not as implemented code):**
  1. Cache the standard-leaf result set once (the host CPUID is already read once at `vmm.cpp:200`
     to learn `highest_basic_leaf`). For the leaves Flex probes, return the cached EAX/ECX/EDX
     without a per-exit `__cpuidex`.
  2. Ensure EAX for `leaf 0` reflects `highest_basic_leaf` (`common.h:129`) so the
     "highest supported basic leaf" value is consistent across exits.
- **State clearly:** this is the *goal*; the handler in `vmexit_handler.cpp:522-574` is the exact
  code point to modify. Do **not** show a full rewritten handler in the doc — describe the change
  and point at the line range.

### 3c. TSC-offset option (secondary, only if RDTSC is *not* untrapped)

- **Where:** the VMCS has `CONTROL_TSC_OFFSET` (`vmcs_encodings.h:168`) and
  `CONTROL_TSC_MULTIPLIER` (`vmcs_encodings.h:185`); `use_tsc_offsetting` is currently **off**
  (`vmcs.cpp:30`).
- **Intended design (state as intent):** when RDTSC is left trapped, enable
  `use_tsc_offsetting = true`, set `CONTROL_TSC_OFFSET` at VM-entry setup (after the CPUID probe /
  on first entry) to hide the pre-existing offset delta, and have the RDTSC handler
  (`vmexit_handler.cpp:1209-1226`) compute a *synthetic* TSC value (`CONTROL_TSC_OFFSET` + measured
  elapsed) rather than returning raw `__rdtsc()` (`:1213`).
- **Position in doc:** present this as the *alternative* to 3a, not the default.

> **Writer note on order in the LaTeX:** lead with 3a (cheap, matches existing `_MINIMAL`), then
> 3b (the real CPUID-cheapening), then 3c (offset alternative). One short paragraph each plus the
> §2 table.

---

## 4. TSC capture (what must be recorded, and why)

State that to verify the fix and to understand the leak, the following must be captured from a
run (this defines the "capture" the reader/writer is describing):

1. **Flex bracket timing:** the two RDTSC values bracketing the CPUID, and the delta. Record both
   the raw values and the computed delta. (This is Flex's own output.)
2. **Exit counts:** number of VM exits taken by the bracket, classified by exit reason. Specifically
   count `EXIT_REASON_CPUID` and `EXIT_REASON_EXCEPTION_NMI` (RDTSC) — i.e. confirm whether 3a took
   effect (RDTSC exits should drop to 0; only CPUID remains).
3. **TSC values at entry/exit:** the guest RAX value read by the RDTSC handler
   (`vmexit_handler.cpp:1213`) and the host `__rdtsc()` value at that instant, for each bracket —
   to measure the offset the handler exposes.
4. **CPUID state:** `highest_basic_leaf` (`common.h:129`) and the EAX/ECX/EDX returned for leaf 0
   and the leaves Flex probes.

**Why each matters (state briefly):** (1) is the observable leak; (2) proves the RDTSC-exit removal
worked; (3) shows whether the TSC delta Flex sees is coming from the offset or from the exits;
(4) confirms the cached CPUID result is what Flex is timing against.

> **Writer note:** this is a *capture spec* — a list of quantities to log, each tied to a code
> point. Do not invent a logging format; just name the quantities and the file:line that produces
> or reads them.

---

## 5. Constraints / "do NOT" list (writer must respect)

- Do not claim CPUID is "made to not exit" — in this build CPUID trapping is unconditional
  (`ia32\vmcs.h:39-60`). The fix is to *cheapen* and *de-redundantize*, not to untrap CPUID.
- Do not present `use_tsc_offsetting` as currently enabled — it is `false` (`vmcs.cpp:30`).
- Do not present the RDTSC handler as returning a synthetic value — it returns raw host TSC
  (`vmexit_handler.cpp:1213`).
- Do not add new function names / constants / bit positions not listed in §2.
- Do not write the fix as already-implemented. Use "intended design" / "change" framing, citing the
  exact file:line to modify.

---

## 6. Suggested LaTeX structure (writer)

1. **Problem:** two-exit CPUID-bracket leak (cite §1, §2 table).
2. **Root causes** (short): unconditional CPUID trap + redundant RDTSC trap + raw-TSC RDTSC handler.
3. **Fix (intended design):** 3a → 3b → 3c, each 1 paragraph + the line range to modify.
4. **TSC capture:** the 4 quantities from §4, each tied to its code point.
5. **Verification:** the observable signature (RDTSC exits → 0; bracket delta unchanged or lower;
   CPUID still returns consistent leaf values).
