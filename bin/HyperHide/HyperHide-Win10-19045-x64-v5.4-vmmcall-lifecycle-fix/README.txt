HyperHide Win10 19045 x64 — v5.4 VMCALL lifecycle fix (PROMPT-03)

Step 1 result: airhv owns VMXOFF.
  airhv/main.cpp driver_unload:14-22 — if vmm_launched then unhook_all + hvgt::vmoff(),
  then disable_vmx_operation / free_vmm_context.
  HyperHideDrv does NOT call hv::vmoff() on unload.

What changed
- VmcallState ACTIVE/DRAINING/OFFLINE + per-CPU VmcallInflight
- Gates inside every broadcast_* DPC callback (not only callers)
- Unload: DRAINING -> drain -> unhook -> OFFLINE (no vmoff)
- LifecycleActive kept in sync via set_vmcall_state
- airhv.sys unchanged: C4F55AD0351B42CB6660538197AD3A7D9E7075676EC0920AC7A5900227E3DD6F

HyperHideDrv.sys SHA-256: FE41B4D884FDDC4032A8E7431192F1388B7FD3AA3B5E1FD7448C1EBAAEF320E3
HyperHideDrv.pdb SHA-256: 6F3F53D852A040E81850EA3B767EE06EC12457B9D7F80DF0291DDEA92CCDCBD4

Bench still required: 20+ stop/start cycles. Do not load on build PC.
