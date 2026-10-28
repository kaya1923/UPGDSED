// Memory.h
#pragma once
#ifndef __MEMORY_H__
#define __MEMORY_H__

#include "Types.h"

#ifdef __cplusplus
extern "C" {
#endif

// Function prototypes for memory-related operations
LPVOID AllocateProcessMemory(HANDLE hProcess, SIZE_T dwSize, DWORD dwAllocationType, DWORD dwProtect);
BOOL FreeProcessMemory(HANDLE hProcess, LPVOID lpBaseAddress);
BOOL ReadProcessMemoryData(HANDLE hProcess, LPCVOID lpBaseAddress, LPVOID lpBuffer, SIZE_T dwSize, SIZE_T* lpNumberOfBytesRead);
BOOL WriteProcessMemoryData(HANDLE hProcess, LPVOID lpBaseAddress, LPCVOID lpBuffer, SIZE_T dwSize, SIZE_T* lpNumberOfBytesWritten);
BOOL ChangeMemoryProtection(HANDLE hProcess, LPVOID lpBaseAddress, SIZE_T dwSize, DWORD dwNewProtect, PDWORD lpOldProtect);
BOOL GetMemoryRegionInfo(LPVOID lpAddress, PMEMORY_REGION pMemoryRegion);
BOOL GetProcessMemoryRegionInfo(HANDLE hProcess, LPVOID lpAddress, PMEMORY_REGION pMemoryRegion);
DWORD EnumerateProcessMemoryRegions(HANDLE hProcess, PMEMORY_REGION* ppMemoryRegionArray, DWORD dwMaxRegions);
BOOL DumpProcessMemory(HANDLE hProcess, LPVOID lpBaseAddress, SIZE_T dwSize, LPCWSTR lpFileName);

#ifdef __cplusplus
}
#endif

#endif // __MEMORY_H__