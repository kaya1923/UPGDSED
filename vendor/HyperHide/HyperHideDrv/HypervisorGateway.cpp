#pragma warning( disable : 4201)
#include <ntifs.h>
#include <intrin.h>
#include "vmintrin.h"
#include "Ntapi.h"
#include "Log.h"
#include "GlobalData.h"

#define IOCTL_POOL_MANAGER_ALLOCATE CTL_CODE(FILE_DEVICE_UNKNOWN, 0x900, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)

extern HYPER_HIDE_GLOBAL_DATA g_HyperHide;

enum vm_call_reasons
{
	VMCALL_TEST,
	VMCALL_VMXOFF,
	VMCALL_EPT_HOOK_FUNCTION,
	VMCALL_EPT_UNHOOK_FUNCTION,
	VMCALL_DUMP_POOL_MANAGER,
	VMCALL_DUMP_VMCS_STATE,
	VMCALL_HIDE_HV_PRESENCE,
	VMCALL_UNHIDE_HV_PRESENCE
};

enum invept_type
{
	INVEPT_SINGLE_CONTEXT = 1,
	INVEPT_ALL_CONTEXTS = 2
};

namespace hv
{
	static ULONG CurrentCpuIndex()
	{
		const ULONG cpu = KeGetCurrentProcessorNumberEx(NULL);
		return (cpu < MAXIMUM_PROCESSORS) ? cpu : 0;
	}

	static VOID InflightEnter()
	{
		InterlockedIncrement(&g_HyperHide.VmcallInflight[CurrentCpuIndex()]);
	}

	static VOID InflightLeave()
	{
		InterlockedDecrement(&g_HyperHide.VmcallInflight[CurrentCpuIndex()]);
	}

	static LONG ReadVmcallState()
	{
		KeMemoryBarrier();
		return InterlockedCompareExchange(&g_HyperHide.VmcallState, 0, 0);
	}

	void set_vmcall_state(LONG NewState)
	{
		InterlockedExchange(&g_HyperHide.VmcallState, NewState);
		KeMemoryBarrier();
		const UCHAR active = (NewState == VMCALL_STATE_ACTIVE) ? 1 : 0;
		InterlockedExchange8(reinterpret_cast<volatile CHAR*>(&g_HyperHide.LifecycleActive), static_cast<CHAR>(active));
	}

	void wait_for_vmcall_drain()
	{
		LARGE_INTEGER delay;
		delay.QuadPart = -1000000LL; // 100ms

		for (ULONG attempt = 0; attempt < 50; ++attempt) // ~5s max
		{
			BOOLEAN busy = FALSE;
			for (ULONG i = 0; i < MAXIMUM_PROCESSORS; ++i)
			{
				if (InterlockedCompareExchange(&g_HyperHide.VmcallInflight[i], 0, 0) != 0)
				{
					busy = TRUE;
					break;
				}
			}

			if (busy == FALSE)
				return;

			LogInfo("still in flight vmcall at unload (attempt %lu)", attempt);
			KeDelayExecutionThread(KernelMode, FALSE, &delay);
		}

		LogError("vmcall drain timed out; continuing unload");
	}

	void broadcast_vmoff(KDPC*, PVOID, PVOID SystemArgument1, PVOID SystemArgument2)
	{
		// Teardown path: only after drain (caller must ensure inflight==0).
		InflightEnter();
		const LONG state = ReadVmcallState();
		if (state != VMCALL_STATE_OFFLINE)
		{
			__vm_call(VMCALL_VMXOFF, 0, 0, 0);
		}
		InflightLeave();
		KeSignalCallDpcSynchronize(SystemArgument2);
		KeSignalCallDpcDone(SystemArgument1);
	}

	struct HookFunctionArgs
	{
		void* target_address;
		void* hook_function;
		void** origin_function;
		unsigned __int64 current_cr3;
		volatile SHORT statuses;
	};

	void broadcast_hook_function(KDPC*, PVOID DeferredContext, PVOID SystemArgument1, PVOID SystemArgument2)
	{
		const auto args = reinterpret_cast<HookFunctionArgs*>(DeferredContext);

		InflightEnter();
		const BOOLEAN go = (ReadVmcallState() == VMCALL_STATE_ACTIVE);
		if (go)
		{
			if (__vm_call_ex(VMCALL_EPT_HOOK_FUNCTION, (unsigned __int64)args->target_address,
				(unsigned __int64)args->hook_function, (unsigned __int64)args->origin_function, args->current_cr3, 0, 0, 0, 0, 0))
			{
				InterlockedIncrement16(&args->statuses);
			}
		}
		InflightLeave();

		KeSignalCallDpcSynchronize(SystemArgument2);
		KeSignalCallDpcDone(SystemArgument1);
	}

	struct UnHookFunctionArgs
	{
		bool unhook_all_functions;
		void* function_to_unhook;
		unsigned __int64 current_cr3;
		volatile SHORT statuses;
	};

	void broadcast_unhook_function(KDPC*, PVOID DeferredContext, PVOID SystemArgument1, PVOID SystemArgument2)
	{
		const auto args = reinterpret_cast<UnHookFunctionArgs*>(DeferredContext);

		InflightEnter();
		// Allow unhook while DRAINING; block only when already OFFLINE.
		const BOOLEAN go = (ReadVmcallState() != VMCALL_STATE_OFFLINE);
		if (go)
		{
			if (__vm_call(VMCALL_EPT_UNHOOK_FUNCTION, args->unhook_all_functions,
				(unsigned __int64)args->function_to_unhook, args->current_cr3))
			{
				InterlockedIncrement16(&args->statuses);
			}
		}
		InflightLeave();

		KeSignalCallDpcSynchronize(SystemArgument2);
		KeSignalCallDpcDone(SystemArgument1);
	}

	void broadcast_test_vmcall(KDPC*, PVOID DeferredContext, PVOID SystemArgument1, PVOID SystemArgument2)
	{
		const auto statuses = reinterpret_cast<volatile SHORT*>(DeferredContext);

		InflightEnter();
		const BOOLEAN go = (ReadVmcallState() == VMCALL_STATE_ACTIVE);
		if (go)
		{
			if (__vm_call(VMCALL_TEST, 0, 0, 0))
			{
				InterlockedIncrement16(statuses);
			}
		}
		InflightLeave();

		KeSignalCallDpcSynchronize(SystemArgument2);
		KeSignalCallDpcDone(SystemArgument1);
	}

	void vmoff()
	{
		KeGenericCallDpc(broadcast_vmoff, NULL);
	}

	void hypervisor_visible(bool value)
	{
		InflightEnter();
		const BOOLEAN go = (ReadVmcallState() == VMCALL_STATE_ACTIVE) && (g_HyperHide.LifecycleActive != 0);
		if (go == FALSE)
		{
			LogInfo("hypervisor_visible skipped: driver lifecycle not active");
			InflightLeave();
			return;
		}

		if (value == true)
			__vm_call(VMCALL_UNHIDE_HV_PRESENCE, 0, 0, 0);
		else
			__vm_call(VMCALL_HIDE_HV_PRESENCE, 0, 0, 0);

		InflightLeave();
	}

	bool unhook_all_functions()
	{
		UnHookFunctionArgs args{ true, nullptr, __readcr3(), 0 };
		KeGenericCallDpc(broadcast_unhook_function, &args);

		return static_cast<ULONG>(args.statuses) == KeQueryActiveProcessorCountEx(ALL_PROCESSOR_GROUPS);
	}

	bool unhook_function(void* function_address)
	{
		UnHookFunctionArgs args{ false, function_address, __readcr3(), 0 };
		KeGenericCallDpc(broadcast_unhook_function, &args);

		return static_cast<ULONG>(args.statuses) == KeQueryActiveProcessorCountEx(ALL_PROCESSOR_GROUPS);
	}

	bool hook_function(void* target_address, void* hook_function, void** origin_function)
	{
		HookFunctionArgs args{ target_address, hook_function, origin_function, __readcr3(), 0 };
		KeGenericCallDpc(broadcast_hook_function, &args);

		return static_cast<ULONG>(args.statuses) == KeQueryActiveProcessorCountEx(ALL_PROCESSOR_GROUPS);
	}

	void dump_pool_manager()
	{
		InflightEnter();
		const BOOLEAN go = (ReadVmcallState() == VMCALL_STATE_ACTIVE) && (g_HyperHide.LifecycleActive != 0);
		if (go == FALSE)
		{
			LogInfo("dump_pool_manager skipped: driver lifecycle not active");
			InflightLeave();
			return;
		}

		__vm_call(VMCALL_DUMP_POOL_MANAGER, 0, 0, 0);
		InflightLeave();
	}

	bool test_vmcall()
	{
		// Zero-init leaves VmcallState=ACTIVE so DriverEntry bootstrap DPCs can run
		// before LifecycleActive is opened for IOCTL paths.
		volatile SHORT statuses{};
		KeGenericCallDpc(broadcast_test_vmcall, (PVOID)&statuses);

		return static_cast<ULONG>(statuses) == KeQueryActiveProcessorCountEx(ALL_PROCESSOR_GROUPS);
	}

	bool send_irp_perform_allocation()
	{
		PDEVICE_OBJECT airhv_device_object;
		KEVENT event;
		PIRP irp;
		IO_STATUS_BLOCK io_status = { 0 };
		UNICODE_STRING airhv_name;
		PFILE_OBJECT file_object;

		RtlInitUnicodeString(&airhv_name, L"\\Device\\airhv");

		NTSTATUS status = IoGetDeviceObjectPointer(&airhv_name, 0, &file_object, &airhv_device_object);

		ObReferenceObjectByPointer(airhv_device_object, FILE_ALL_ACCESS, NULL, KernelMode);

		ObDereferenceObject(file_object);

		if (NT_SUCCESS(status) == false)
		{
			LogError("Couldn't get hypervisor device object pointer");
			return false;
		}

		KeInitializeEvent(&event, NotificationEvent, 0);
		irp = IoBuildDeviceIoControlRequest(IOCTL_POOL_MANAGER_ALLOCATE, airhv_device_object, 0, 0, 0, 0, 0, &event, &io_status);

		if (irp == NULL)
		{
			LogError("Couldn't create Irp");
			ObDereferenceObject(airhv_device_object);
			return false;
		}

		else
		{
			status = IofCallDriver(airhv_device_object, irp);

			if (status == STATUS_PENDING)
				KeWaitForSingleObject(&event, Executive, KernelMode, 0, 0);

			ObDereferenceObject(airhv_device_object);
			return true;
		}
	}
}
