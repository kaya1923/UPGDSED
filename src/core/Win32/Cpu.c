// Cpu.c
#include "Cpu.h"

#pragma warning(disable: 4100) // Unreferenced parameter

// Function to get CPU information
BOOL GetCpuInfo(PCPU_INFO pCpuInfo) {
    SYSTEM_INFO si;
    
    GetSystemInfo(&si);

    // This function provides basic CPU info, for more advanced features like 
    // Processor Manufacturer or Performance Counters one could use 
    // GetSystemTimes() or performance counters
    
    pCpuInfo->dwNumberOfProcessors = si.dwNumberOfProcessors;
    pCpuInfo->wProcessorArchitecture = si.wProcessorArchitecture;
    pCpuInfo->wProcessorLevel = si.wProcessorLevel;
    pCpuInfo->wProcessorRevision = si.wProcessorRevision;

    return TRUE;
}

// Function to check if the system supports a particular feature (e.g., SSE)
BOOL IsProcessorFeaturePresentEx(DWORD dwFeature) {
    return IsProcessorFeaturePresent(dwFeature);
}

// Function to get processor performance counter frequency 
BOOL GetProcessorFrequency(PDWORD pdwFrequency) {
    LARGE_INTEGER liFrequency;
    
    if (!QueryPerformanceFrequency(&liFrequency)) {
        return FALSE;
    }
    
    *pdwFrequency = (DWORD)liFrequency.QuadPart;
    return TRUE;
}

// Function to get the current CPU usage by percentage
BOOL GetCpuUsage(PFLOAT pfUsage) {
    static DWORD lastTotalTime = 0;
    DWORD currentTime;
    FILETIME ftime, fsys, fuser;
    static DWORD lastSysTotalTime = 0;
    static DWORD lastUserTotalTime = 0;
    DWORD sysTotalTime, userTotalTime;
    DWORD totalCpuTime;

    // Get current system & user CPU times
    GetSystemTimes(&ftime, &fsys, &fuser);

    // Convert FILETIMEs to integer values
    currentTime = (DWORD)(ftime.dwLowDateTime | ((ULONGLONG)ftime.dwHighDateTime << 32));
    sysTotalTime = (DWORD)(fsys.dwLowDateTime | ((ULONGLONG)fsys.dwHighDateTime << 32));
    userTotalTime = (DWORD)(fuser.dwLowDateTime | ((ULONGLONG)fuser.dwHighDateTime << 32));

    // Calculate CPU usage as a percentage (based on total time since last measurement)
    totalCpuTime = sysTotalTime + userTotalTime - lastSysTotalTime - lastUserTotalTime; 
    *pfUsage = (float)totalCpuTime / (float)(currentTime - lastTotalTime) * 100;
    
    // Update for next measurement
    lastTotalTime = currentTime;
    lastSysTotalTime = sysTotalTime;
    lastUserTotalTime = userTotalTime;

    return TRUE;
}
