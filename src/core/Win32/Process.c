// Process.c
#include "Process.h"
#include "Utils.h"
#include <psapi.h>

#pragma warning(disable: 4100) // Unreferenced parameter

// Function to get process information by name
BOOL GetProcessInfoByName(LPCWSTR lpProcessName, PPROCESS_INFO pProcessInfo) {
    DWORD dwProcessIds[1024];
    DWORD dwBytesReturned;
    DWORD dwProcessCount;
    HANDLE hSnapshot;
    PROCESSENTRY32 pe32;

    // Take a snapshot of all processes in the system
    hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE)
        return FALSE;

    // Set the size of the structure before using it
    pe32.dwSize = sizeof(PROCESSENTRY32);

    // Retrieve information about the first process and exit if unsuccessful
    if (!Process32First(hSnapshot, &pe32)) {
        CloseHandle(hSnapshot);
        return FALSE;
    }

    // Walk the snapshot of processes
    do {
        // Compare the name of the process with the provided one (case insensitive)
        if (_wcsicmp(pe32.szExeFile, lpProcessName) == 0) {
            pProcessInfo->dwProcessId = pe32.th32ProcessID;
            pProcessInfo->dwParentProcessId = pe32.th32ParentProcessID;
            pProcessInfo->lpProcessName = _wcsdup(pe32.szExeFile);
            pProcessInfo->hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID);
            
            // Check if it's a 64-bit process
            pProcessInfo->bIs64Bit = IsProcess64Bit(pProcessInfo->hProcess);
            pProcessInfo->bHasDebugPrivilege = HasDebugPrivilege();

            CloseHandle(hSnapshot);
            return TRUE;
        }
    } while (Process32Next(hSnapshot, &pe32));

    // No process found
    CloseHandle(hSnapshot);
    return FALSE;
}

// Function to check if a given process is running 64-bit
BOOL IsProcess64Bit(HANDLE hProcess) {
    BOOL bIsWow64 = FALSE;
    BOOL bSuccess = IsWow64Process(hProcess, &bIsWow64);

    // If IsWow64Process failed, assume it's not 64-bit
    if (!bSuccess) {
        return FALSE;
    }

    // If process is running under WOW64, it is 32-bit on a 64-bit system
    if (bIsWow64) {
        return FALSE;  // 32-bit process 
    }

    // Otherwise, check using GetNativeSystemInfo or another method to determine if it is truly 64-bit
    SYSTEM_INFO si;
    GetNativeSystemInfo(&si);

    // If native system info indicates we're on a 64-bit machine and the process isn't under WOW64,
    // then we might need to do further checks to determine actual architecture
    return TRUE;  // Assume 64-bit if not under WOW64
}

// Function to get all running processes
DWORD GetRunningProcesses(PPROCESS_INFO* ppProcessInfoArray) {
    DWORD dwIndex = 0;
    DWORD dwProcessIds[1024];
    DWORD dwBytesReturned;
    DWORD dwProcessCount;
    HANDLE hSnapshot;
    PROCESSENTRY32 pe32;

    // Take a snapshot of all processes in the system
    hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE)
        return 0;

    // Set the size of the structure before using it
    pe32.dwSize = sizeof(PROCESSENTRY32);

    // Retrieve information about the first process and exit if unsuccessful
    if (!Process32First(hSnapshot, &pe32)) {
        CloseHandle(hSnapshot);
        return 0;
    }

    // Allocate memory for the array of processes
    *ppProcessInfoArray = (PPROCESS_INFO)malloc(sizeof(PROCESS_INFO) * 1024);
    if (*ppProcessInfoArray == NULL) {
        CloseHandle(hSnapshot);
        return 0;
    }

    // Walk the snapshot of processes
    do {
        (*ppProcessInfoArray)[dwIndex].dwProcessId = pe32.th32ProcessID;
        (*ppProcessInfoArray)[dwIndex].dwParentProcessId = pe32.th32ParentProcessID;
        (*ppProcessInfoArray)[dwIndex].lpProcessName = _wcsdup(pe32.szExeFile);
        (*ppProcessInfoArray)[dwIndex].hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID);
        
        // Check if it's a 64-bit process
        (*ppProcessInfoArray)[dwIndex].bIs64Bit = IsProcess64Bit((*ppProcessInfoArray)[dwIndex].hProcess);
        (*ppProcessInfoArray)[dwIndex].bHasDebugPrivilege = HasDebugPrivilege();

        dwIndex++;
    } while (Process32Next(hSnapshot, &pe32) && dwIndex < 1024);

    CloseHandle(hSnapshot);
    return dwIndex;
}

// Function to kill a process by its ID
BOOL KillProcess(DWORD dwProcessId) {
    HANDLE hProcess;

    // Open the process with termination access right
    hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, dwProcessId);
    if (hProcess == NULL)
        return FALSE;

    // Terminate the process
    BOOL bResult = TerminateProcess(hProcess, 0);

    // Close the handle to the process
    CloseHandle(hProcess);
    
    return bResult;
}

// Function to check if a process has debug privilege
BOOL HasDebugPrivilege() {
    HANDLE hToken;
    TOKEN_PRIVILEGES tp;
    BOOL bResult = FALSE;

    // Open the process token of the current process
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
        // Get the LUID for the debug privilege
        if (LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &tp.Privileges[0].Luid)) {
            tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
            tp.PrivilegeCount = 1;

            // Enable the debug privilege
            bResult = AdjustTokenPrivileges(hToken, FALSE, &tp, 0, NULL, NULL);
        }
        CloseHandle(hToken);
    }

    return bResult;
}
