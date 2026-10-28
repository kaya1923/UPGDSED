// Process.h
#pragma once
#ifndef __PROCESS_H__
#define __PROCESS_H__

#include "Types.h"

#ifdef __cplusplus
extern "C" {
#endif

// Function prototypes for process-related operations
BOOL GetProcessInfoByName(LPCWSTR lpProcessName, PPROCESS_INFO pProcessInfo);
BOOL IsProcess64Bit(HANDLE hProcess);
DWORD GetRunningProcesses(PPROCESS_INFO* ppProcessInfoArray);
BOOL KillProcess(DWORD dwProcessId);
BOOL HasDebugPrivilege();

#ifdef __cplusplus
}
#endif

#endif // __PROCESS_H__