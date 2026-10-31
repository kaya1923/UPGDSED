# PROMPT-03 — VMCALL lifecycle fix → v5.4

Date: 2026-10-31 (build PC). Driver not loaded here.

## Step 1 — Who owns VMXOFF?

**airhv** owns VMXOFF / VMCS teardown.

Evidence (`vendor/HyperHide/airhv/airhv/main.cpp`):

```text
driver_unload (lines 14-32):
  if g_vmm_context && vcpu_table[0]->vmm_launched:
      hvgt::unhook_all_functions();
      hvgt::vmoff();          // VMCALL_VMXOFF broadcast
  hv::disable_vmx_operation();
  free_vmm_context();
```

`airhv/hypervisor_gateway.cpp` `vmoff()` → `broadcast_vmoff` → `__vm_call(VMCALL_VMXOFF,…)`.
`airhv/vmcall_handler.cpp` case `VMCALL_VMXOFF` → `call_vmxoff()` → `__vmx_off()`.

Therefore **HyperHideDrv must not call `hv::vmoff()` on its unload** while airhv remains
the hypervisor owner. HyperHide only drains + EPT-unhooks while the VMCS is still live.

## What changed

| Area | Change |
|------|--------|
| `GlobalData.h` | `VmcallState` ACTIVE/DRAINING/OFFLINE + `VmcallInflight[MAXIMUM_PROCESSORS]`; keep `LifecycleActive` synced |
| `HypervisorGateway.cpp` | Gate **inside** every `broadcast_*` DPC; direct paths use same state+inflight |
| `Driver.cpp` unload | DRAINING → drain → `unhook_all_functions` → OFFLINE → device teardown; **no vmoff** |
| ABI | `vmintrin.asm` untouched |

## Artifact

- ZIP SHA-256: `F0AF68D2A53F2E66FB678208408D84461172BA48962F57369496C8C53A70FD55`
- `HyperHideDrv.sys` SHA-256: `FE41B4D884FDDC4032A8E7431192F1388B7FD3AA3B5E1FD7448C1EBAAEF320E3`
- Path: `bin/HyperHide/HyperHide-Win10-19045-x64-v5.4-vmmcall-lifecycle-fix.zip`
- `airhv.sys` unchanged `C4F55AD0…`
- NtQueryObject offline tests: passed

## Tests

| Test | Status |
|------|--------|
| Release x64 build + PDB | done |
| NtQueryObject safety offline | passed |
| HP-Bench 20+ stop/start | **not run** |
| Fast attach/detach / 4-CPU | **not run** |
