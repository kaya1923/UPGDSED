// Memory.c
#include "Memory.h"

#pragma warning(disable: 4100) // Unreferenced parameter

// Function to allocate memory in a target process
LPVOID AllocateProcessMemory(HANDLE hProcess, SIZE_T dwSize, DWORD dwAllocationType, DWORD dwProtect) {
    return VirtualAllocEx(hProcess, NULL, dwSize, dwAllocationType, dwProtect);
}

// Function to free memory in a target process
BOOL FreeProcessMemory(HANDLE hProcess, LPVOID lpBaseAddress) {
    return VirtualFreeEx(hProcess, lpBaseAddress, 0, MEM_RELEASE);
}

// Function to read memory from a target process
BOOL ReadProcessMemoryData(HANDLE hProcess, LPCVOID lpBaseAddress, LPVOID lpBuffer, SIZE_T dwSize, SIZE_T* lpNumberOfBytesRead) {
    return ReadProcessMemory(hProcess, lpBaseAddress, lpBuffer, dwSize, lpNumberOfBytesRead);
}

// Function to write memory in a target process
BOOL WriteProcessMemoryData(HANDLE hProcess, LPVOID lpBaseAddress, LPCVOID lpBuffer, SIZE_T dwSize, SIZE_T* lpNumberOfBytesWritten) {
    return WriteProcessMemory(hProcess, lpBaseAddress, lpBuffer, dwSize, lpNumberOfBytesWritten);
}

// Function to change memory protection of a region in a target process
BOOL ChangeMemoryProtection(HANDLE hProcess, LPVOID lpBaseAddress, SIZE_T dwSize, DWORD dwNewProtect, PDWORD lpOldProtect) {
    return VirtualProtectEx(hProcess, lpBaseAddress, dwSize, dwNewProtect, lpOldProtect);
}

// Function to get memory information for a specified address in the current process
BOOL GetMemoryRegionInfo(LPVOID lpAddress, PMEMORY_REGION pMemoryRegion) {
    MEMORY_BASIC_INFORMATION mbi;
    
    if (VirtualQuery(lpAddress, &mbi, sizeof(MEMORY_BASIC_INFORMATION)) == 0) {
        return FALSE;
    }

    pMemoryRegion->lpBaseAddress = mbi.BaseAddress;
    pMemoryRegion->dwRegionSize = mbi.RegionSize;
    pMemoryRegion->dwState = mbi.State;
    pMemoryRegion->dwProtect = mbi.Protect;
    pMemoryRegion->dwType = mbi.Type;

    return TRUE;
}

// Function to get memory information for a specified address in a target process
BOOL GetProcessMemoryRegionInfo(HANDLE hProcess, LPVOID lpAddress, PMEMORY_REGION pMemoryRegion) {
    MEMORY_BASIC_INFORMATION mbi;
    
    if (VirtualQueryEx(hProcess, lpAddress, &mbi, sizeof(MEMORY_BASIC_INFORMATION)) == 0) {
        return FALSE;
    }

    pMemoryRegion->lpBaseAddress = mbi.BaseAddress;
    pMemoryRegion->dwRegionSize = mbi.RegionSize;
    pMemoryRegion->dwState = mbi.State;
    pMemoryRegion->dwProtect = mbi.Protect;
    pMemoryRegion->dwType = mbi.Type;

    return TRUE;
}

// Function to iterate through all memory regions in a process
DWORD EnumerateProcessMemoryRegions(HANDLE hProcess, PMEMORY_REGION* ppMemoryRegionArray, DWORD dwMaxRegions) {
    MEMORY_BASIC_INFORMATION mbi;
    LPVOID lpAddress = 0;
    DWORD dwIndex = 0;

    // Allocate memory for the array of memory regions
    *ppMemoryRegionArray = (PMEMORY_REGION)malloc(sizeof(MEMORY_REGION) * dwMaxRegions);
    if (*ppMemoryRegionArray == NULL)
        return 0;

    while (dwIndex < dwMaxRegions && VirtualQueryEx(hProcess, lpAddress, &mbi, sizeof(MEMORY_BASIC_INFORMATION)) != 0) {
        // Only include committed regions
        if (mbi.State & MEM_COMMIT) {
            (*ppMemoryRegionArray)[dwIndex].lpBaseAddress = mbi.BaseAddress;
            (*ppMemoryRegionArray)[dwIndex].dwRegionSize = mbi.RegionSize;
            (*ppMemoryRegionArray)[dwIndex].dwState = mbi.State;
            (*ppMemoryRegionArray)[dwIndex].dwProtect = mbi.Protect;
            (*ppMemoryRegionArray)[dwIndex].dwType = mbi.Type;

            dwIndex++;
        }

        // Move to next region
        lpAddress = (LPVOID)((BYTE*)mbi.BaseAddress + mbi.RegionSize);
    }

    return dwIndex;
}

// Function to dump memory from a process 
BOOL DumpProcessMemory(HANDLE hProcess, LPVOID lpBaseAddress, SIZE_T dwSize, LPCWSTR lpFileName) {
    HANDLE hFile;
    DWORD dwBytesWritten;

    // Create output file
    hFile = CreateFileW(lpFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
        return FALSE;

    // Read memory from the process into a buffer
    LPVOID lpBuffer = malloc(dwSize);
    if (lpBuffer == NULL) {
        CloseHandle(hFile);
        return FALSE;
    }

    if (!ReadProcessMemoryData(hProcess, lpBaseAddress, lpBuffer, dwSize, NULL)) {
        free(lpBuffer);
        CloseHandle(hFile);
        return FALSE;
    }

    // Write to file
    BOOL bResult = WriteFile(hFile, lpBuffer, dwSize, &dwBytesWritten, NULL);

    // Cleanup
    free(lpBuffer);
    CloseHandle(hFile);

    return bResult;
}
