// System.h
#pragma once
#ifndef __SYSTEM_H__
#define __SYSTEM_H__

#include "Types.h"

#ifdef __cplusplus
extern "C" {
#endif

// Function prototypes for system-related operations
BOOL GetSystemInfoEx(PSYSTEM_INFO pSystemInfo);
DWORD GetCurrentProcessIdEx();
DWORD GetCurrentThreadIdEx();
BOOL IsWindows64Bit();
BOOL GetAvailableMemory(PDWORD pdwTotalPhysicalMemory, PDWORD pdwAvailablePhysicalMemory);
BOOL AdjustDebugPrivilege(BOOL bEnable);

#ifdef __cplusplus
}
#endif

#endif // __SYSTEM_H__