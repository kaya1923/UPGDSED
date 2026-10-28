// System.c
#include "System.h"

#pragma warning(disable: 4100) // Unreferenced parameter

// Function to get system information
BOOL GetSystemInfoEx(PSYSTEM_INFO pSystemInfo) {
    SYSTEM_INFO si;

    GetSystemInfo(&si);

    pSystemInfo->dwProcessorCount = si.dwNumberOfProcessors;
    pSystemInfo->dwPageSize = si.dwPageSize;
    pSystemInfo->lpMinimumApplicationAddress = si.lpMinimumApplicationAddress;
    pSystemInfo->lpMaximumApplicationAddress = si.lpMaximumApplicationAddress;
    pSystemInfo->dwActiveProcessorMask = si.dwActiveProcessorMask;
    pSystemInfo->dwNumberOfProcessors = si.dwNumberOfProcessors;
    pSystemInfo->dwProcessorType = si.dwProcessorType;
    pSystemInfo->dwAllocationGranularity = si.dwAllocationGranularity;
    pSystemInfo->wProcessorLevel = si.wProcessorLevel;
    pSystemInfo->wProcessorRevision = si.wProcessorRevision;

    return TRUE;
}

// Function to get the current process ID
DWORD GetCurrentProcessIdEx() {
    return GetCurrentProcessId();
}

// Function to get the current thread ID
DWORD GetCurrentThreadIdEx() {
    return GetCurrentThreadId();
}

// Function to check if running on a 64-bit operating system
BOOL IsWindows64Bit() {
    BOOL bIsWow64 = FALSE;
    
    // Use IsWow64Process function to check if we're running in WoW64 mode (32-bit process on 64-bit OS)
    if (!IsWow64Process(GetCurrentProcess(), &bIsWow64)) {
        return FALSE;
    }
    
    // If the system is not 64-bit OR this process is WoW64, we're not on native 64-bit
    // The actual check for whether it's a 64-bit OS would require checking GetNativeSystemInfo 
    if (!bIsWow64) {
        SYSTEM_INFO si;
        GetNativeSystemInfo(&si);
        
        // If the architecture is IA64 or x64, then we're running on 64-bit OS
        return (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 || 
                si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64);
    }
    
    return TRUE; // Otherwise assume 64-bit
}

// Function to get available memory
BOOL GetAvailableMemory(PDWORD pdwTotalPhysicalMemory, PDWORD pdwAvailablePhysicalMemory) {
    MEMORYSTATUSEX ms;
    ms.dwLength = sizeof(MEMORYSTATUSEX);

    if (!GlobalMemoryStatusEx(&ms)) {
        return FALSE;
    }

    *pdwTotalPhysicalMemory = ms.dwTotalPhys;
    *pdwAvailablePhysicalMemory = ms.dwAvailPhys;

    return TRUE;
}

// Function to enable or disable debug privilege
BOOL AdjustDebugPrivilege(BOOL bEnable) {
    HANDLE hToken;
    TOKEN_PRIVILEGES tp;
    BOOL bResult = FALSE;

    // Open the process token of the current process
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
        // Get the LUID for the debug privilege
        if (LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &tp.Privileges[0].Luid)) {
            tp.Privileges[0].Attributes = bEnable ? SE_PRIVILEGE_ENABLED : 0;
            tp.PrivilegeCount = 1;

            // Adjust the token privileges
            bResult = AdjustTokenPrivileges(hToken, FALSE, &tp, 0, NULL, NULL);
        }
        CloseHandle(hToken);
    }

    return bResult;
}
